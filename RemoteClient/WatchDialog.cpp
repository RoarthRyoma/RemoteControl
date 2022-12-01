// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "WatchDialog.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
	, m_nObjWidth(-1)
	, m_nObjHeight(-1)
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
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point, bool isScreen/* =false*/)
{
	CRect clientRect;
	point.y -= 30;
	//全局坐标到客户端坐标
	if(isScreen) ScreenToClient(&point);
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

	SetTimer(0, 50, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull())
		{
			//显示
			CRect rect;
			m_picture.GetWindowRect(rect);
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			auto image = pParent->GetImage();
			if (m_nObjWidth == -1)m_nObjWidth = image.GetWidth();
			if(m_nObjHeight == -1)m_nObjHeight = image.GetHeight();
			image.StretchBlt(
				m_picture.GetDC()->GetSafeHdc(), 0, 0,
				rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			image.Destroy();
			pParent->SetImageStatus();
		}
	}

	CDialog::OnTimer(nIDEvent);
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
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
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
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
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
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
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
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
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
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
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
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
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
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
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
		//CClientSocket* pClient = CClientSocket::getInstance();
		//CPacket pack(5, (BYTE*)&event, sizeof(event));
		//pClient->Send(pack);
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (LPARAM) & event);
	}
}


void CWatchDialog::OnOK()
{
	//CDialog::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);
}
