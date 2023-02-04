// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "WatchDialog.h"
#include "afxdialogex.h"
#include "ClientController.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
	, m_nObjWidth(-1)
	, m_nObjHeight(-1)
	, m_isFull(false)
{

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CWatchDialog::OnSendPackAck)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point, bool isScreen/* =false*/)
{
	CRect clientRect;
	if(!isScreen)ClientToScreen(&point);//转换为相对屏幕左上角的坐标(屏幕内的绝对坐标)
	m_picture.ScreenToClient(&point);//转换为客户区域坐标(相对picture控件左上角的坐标)
	TRACE("x-%d y-%d\r\n", point.x, point.y);
	//本地客户端坐标到远程坐标
	m_picture.GetWindowRect(clientRect);
	TRACE("trans x-%d y-%d\r\n", clientRect.Width(), clientRect.Height());
	//float cwidth = clientRect.Width();
	//float cheight = clientRect.Height();
	//int width = 1920, height = 1080;
	//int x = point.x * width / cwidth;
	//int y = point.y * height / cheight;

	return CPoint(point.x * m_nObjWidth / clientRect.Width(),
		point.y * m_nObjHeight / clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	//CRect   temprect(0, 0, 1920, 1080);
	CRect   temprect(0, 0, 1920, 1140);
	CWatchDialog::SetWindowPos(NULL, 0, 0, temprect.Width(), temprect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	//MoveWindow(&temprect);
	m_isFull = false;
	//SetTimer(0, 50, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	//if (nIDEvent == 0)
	//{
	//	if (this->isFull())
	//	{
	//		//显示
	//		CRect rect;
	//		m_picture.GetWindowRect(rect);
	//		//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
	//		m_nObjWidth = m_image.GetWidth();
	//		m_nObjHeight = m_image.GetHeight();
	//		m_image.StretchBlt(
	//			m_picture.GetDC()->GetSafeHdc(), 0, 0,
	//			rect.Width(), rect.Height(), SRCCOPY);
	//		m_picture.InvalidateRect(NULL);
	//		m_image.Destroy();
	//		this->SetImageStatus();
	//		TRACE("更新图片完成%d %d\r\n", m_nObjWidth, m_nObjHeight);
	//	}
	//}

	CDialog::OnTimer(nIDEvent);
}


LRESULT CWatchDialog::OnSendPackAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || lParam == -2)
	{
		//TODO:错误处理

	}
	else if (lParam == 1)
	{
		//对方关闭了套接字

	}
	else
	{
		if ((CPacket*)wParam != NULL)
		{
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (head.sCmd)
			{
			case 6:
			{
				CEdoyunTool::Byte2Image(m_image, head.strData);
				CRect rect;
				m_picture.GetWindowRect(rect);
				m_nObjWidth = m_image.GetWidth();
				m_nObjHeight = m_image.GetHeight();
				m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
				m_picture.InvalidateRect(NULL);
				TRACE("更新图片完成！%d %d %08X\r\n", m_nObjWidth, m_nObjHeight, (HBITMAP)m_image);
				m_image.Destroy();
				break;
			}
			case 5:
				TRACE("远程端应答了鼠标操作！\r\n");
				break;
			case 7:
			case 8:
			default:
				break;

			}
		}
	}

	return 0;
}

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//坐标转换
		CPoint remotePt = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remotePt;
		event.nButton = 0;//左键
		event.nAction = 1;//双击
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, (BYTE*)&event, sizeof(event), true);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		TRACE("x-%d y-%d\r\n", point.x, point.y);
		//坐标转换
		CPoint remotePt = UserPoint2RemoteScreenPoint(point);
		TRACE("trans => x-%d y-%d\r\n", point.x, point.y);
		//封装
		MOUSEEV event;
		event.ptXY = remotePt;
		event.nButton = 0;//左键
		event.nAction = 2;//按下
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, (BYTE*)&event, sizeof(event), true);
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//坐标转换
		CPoint remotePt = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remotePt;
		event.nButton = 0;//左键
		event.nAction = 3;//弹起
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, (BYTE*)&event, sizeof(event), true);
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//坐标转换
		CPoint remotePt = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remotePt;
		event.nButton = 2;//右键
		event.nAction = 1;//双击
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, (BYTE*)&event, sizeof(event), true);
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//坐标转换
		CPoint remotePt = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remotePt;
		event.nButton = 1;//右键
		event.nAction = 2;//按下 //TODO: 服务要做对应的修改
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, (BYTE*)&event, sizeof(event), true);
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//坐标转换
		CPoint remotePt = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remotePt;
		event.nButton = 1;//右键
		event.nAction = 3;//弹起
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, (BYTE*)&event, sizeof(event), true);
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//坐标转换
		CPoint remotePt = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remotePt;
		event.nButton = 8;//移动时默认
		event.nAction = 0;//移动
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, (BYTE*)&event, sizeof(event), true);
	}
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint point;
		GetCursorPos(&point);
		//坐标转换
		CPoint remotePt = UserPoint2RemoteScreenPoint(point, true);
		//封装
		MOUSEEV event;
		event.ptXY = remotePt;
		event.nButton = 0;//左键
		event.nAction = 0;//单击
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, (BYTE*)&event, sizeof(event), true);
	}
}


void CWatchDialog::OnOK()
{
	//CDialog::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 7);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 8);
}
