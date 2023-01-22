
// RemoteClientDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "ClientController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg dialog



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

//int CRemoteClientDlg::SendCommandPacket(int nCmd, BYTE* pData /*= NULL*/, size_t nLength /*= 0*/, bool bAutoClose /*= true*/)
//{
//	return CClientController::getInstance()->SendCommandPacket(nCmd, pData, nLength, bAutoClose);
//}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMRClickTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	//ON_MESSAGE(WM_SEND_PACKET, &CRemoteClientDlg::OnSendPacket)
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
	ON_WM_TIMER()
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERV, &CRemoteClientDlg::OnIpnFieldchangedIpaddressServ)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CRemoteClientDlg::OnEnChangeEditPort)
END_MESSAGE_MAP()


// CRemoteClientDlg message handlers

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	UpdateData(TRUE);
	//m_server_address = 0x7F000001;//127.0.0.1
	m_server_address = 0xC0A80B81;	//192.168.11.129
	m_port = _T("9527");
	CClientController::getInstance()->UpdateAddress(m_server_address, atoi((LPCTSTR)m_port));
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DLG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);

	//m_isFull = false;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteClientDlg::OnBnClickedBtnTest()
{
	CClientController::getInstance()->SendCommandPacket(1981);
}


void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	int ret = CClientController::getInstance()->SendCommandPacket(1);
	if (ret == -1)
	{
		AfxMessageBox(_T("命令处理失败"));
		return;
	}
	std::string drivers = CClientController::getInstance()->GetPacket().strData;
	drivers += ",";
	std::string dr;
	m_Tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',')
		{
			dr += ":";
			HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			if(hTemp != 0) m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
}

//void CRemoteClientDlg::threadEntryForWatchData(void* arg)
//{
//	CRemoteClientDlg* that = (CRemoteClientDlg*)arg;
//	that->threadWatchData();
//	_endthread();
//}
//
//void CRemoteClientDlg::threadWatchData()
//{
//	Sleep(50);
//	CClientController* pCtrl = CClientController::getInstance();
//	//CClientSocket* pClient = NULL;
//	//do 
//	//{
//	//	pClient = CClientSocket::getInstance();
//	//} while (pClient == NULL);
//	//ULONGLONG tick = GetTickCount64();
//	//for (;;)//等同于while(true)
//	while(!m_isClosed)
//	{
//		//if (GetTickCount64() - tick < 60)//增加间隔
//		//{
//		//	Sleep(GetTickCount64() - tick);
//		//}
//		//CPacket pack(6, NULL, 0);
//		//bool ret = pClient->Send(pack);
//		if (!m_isFull)//数据更新到缓存
//		{
//			int ret = pCtrl->SendCommandPacket(6);
//			//int ret = SendMessage(WM_SEND_PACKET, 6 << 1 | 1);
//			if (ret == 6)
//			{
//				if(pCtrl->GetImage(m_image) == 0)
//				{
//					m_isFull = true;
//				}
//				else
//				{
//					TRACE("获取图片失败！\r\n");
//				}
//			}
//			else
//			{
//				Sleep(1);
//			}
//		}
//		else
//		{
//			Sleep(1);
//		}
//	}
//}

//void CRemoteClientDlg::threadEntryForDownFile(void* arg)
//{
//	CRemoteClientDlg* that = (CRemoteClientDlg*)arg;
//	that->threadDownFile();
//	_endthread();
//}
//
//void CRemoteClientDlg::threadDownFile()
//{
//	int nListSelected = m_List.GetSelectionMark();
//	CString strFile = m_List.GetItemText(nListSelected, 0);
//
//	CFileDialog dlg(FALSE, "*", strFile,
//		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
//		NULL, this);
//	if (dlg.DoModal() == IDOK)
//	{
//		FILE* pFile = NULL;
//		errno_t err = fopen_s(&pFile, dlg.GetPathName(), "wb+");
//		if (err != 0)
//		{
//			AfxMessageBox(_T("没有权限保存该文件或文件无法创建!"));
//			m_dlgStatus.ShowWindow(SW_HIDE);
//			EndWaitCursor();
//			return;
//		}
//		HTREEITEM hSelected = m_Tree.GetSelectedItem();
//		strFile = GetPath(hSelected) + strFile;//根据选中获取完整路径
//		TRACE("File Path: %s\r\n", (LPCTSTR)strFile);
//		CClientSocket* pClient = CClientSocket::getInstance();
//		do
//		{
//			//int ret = SendCommandPacket(4, (BYTE*)(LPCTSTR)strFile, strFile.GetLength(), false);
//			//int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCTSTR)strFile);
//			int ret = CClientController::getInstance()->SendCommandPacket(4, (BYTE*)(LPCTSTR)strFile, strFile.GetLength(), false);
//			if (ret < 0)
//			{
//				AfxMessageBox("下载文件命令执行失败!");
//				TRACE("执行download ret: %d\r\n", ret);
//				//fclose(pFile);
//				//pClient->CloseSocket();
//				//return;
//				break;
//			}
//			long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
//			if (nLength == 0)
//			{
//				AfxMessageBox("文件长度为0或无法读取文件");
//				//return;
//				break;
//			}
//			long long nCount = 0;
//			while (nCount < nLength)
//			{
//				ret = pClient->DealCommand();
//				if (ret < 0)
//				{
//					AfxMessageBox("传输失败!");
//					TRACE("传输失败 ret:%d", ret);
//					break;
//				}
//				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
//				nCount += pClient->GetPacket().strData.size();
//			}
//		} while (false);
//		fclose(pFile);
//		pClient->CloseSocket();
//	}
//	m_dlgStatus.ShowWindow(SW_HIDE);
//	EndWaitCursor();
//	MessageBox(_T("下载完成!! "), _T("完成"));
//}

void CRemoteClientDlg::LoadFileCurrent()
{
	HTREEITEM hTreeSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hTreeSelected);
	m_List.DeleteAllItems();
	CClientController* pCtrl = CClientController::getInstance();
	int nCmd = pCtrl->SendCommandPacket(2, (BYTE*)(LPCTSTR)strPath, strPath.GetLength(), false);
	PFILEINFO pinfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	int cmd;
	while (pinfo->HasNext)
	{
		TRACE("[%s] isdir %d\r\n", pinfo->szFileName, pinfo->IsDirectory);
		if (!pinfo->IsDirectory)
		{
			m_List.InsertItem(0, pinfo->szFileName);
		}
		cmd = pCtrl->DealCommand();
		TRACE("ack: %d\r\n", cmd);
		if (cmd < 0) break;
		pinfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	}

	pCtrl->CloseSocket();
}

void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	UINT uFlag = 0;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, &uFlag);

	//点击空白处直接返回
	if (hTreeSelected == NULL) return;
	//是文件没有子目录直接返回
	if (m_Tree.GetChildItem(hTreeSelected) == NULL) return;

	DeleteTreeChildrenItem(hTreeSelected);
	m_List.DeleteAllItems();

	CString strPath = GetPath(hTreeSelected);
	int nCmd = CClientController::getInstance()->SendCommandPacket(2, (BYTE*)(LPCTSTR)strPath, strPath.GetLength(), false);
	PFILEINFO pinfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	int COUNT = 0;
	int cmd;
	while (pinfo->HasNext)
	{
		TRACE("[%s] isdir %d\r\n", pinfo->szFileName, pinfo->IsDirectory);
		if (pinfo->IsDirectory)
		{
			//排除 . 和 .. 两个目录
			if (CString(pinfo->szFileName) == "." || (CString(pinfo->szFileName) == ".."))
			{
				int cmd = CClientController::getInstance()->DealCommand();
				TRACE("ack: %d\r\n", cmd);
				if (cmd < 0) break;
				pinfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pinfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", hTemp, TVI_LAST);
		}
		else
		{
			m_List.InsertItem(0, pinfo->szFileName);
		}
		//HTREEITEM hTemp = m_Tree.InsertItem(pinfo->szFileName, hTreeSelected, TVI_LAST);
		//if (pinfo->IsDirectory)
		//{
		//	m_Tree.InsertItem("", hTemp, TVI_LAST);
		//}
		COUNT++;
		cmd = CClientController::getInstance()->DealCommand();
		TRACE("ack: %d\r\n", cmd);
		if (cmd < 0) break;
		pinfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	}
	TRACE("client COUNT: %d\r\n", COUNT);
	CClientController::getInstance()->CloseSocket();
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTmp;
	do 
	{
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}

void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do 
	{
		hSub = m_Tree.GetChildItem(hTree);
		if (hSub != NULL) m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LoadFileInfo();
}

void CRemoteClientDlg::OnNMRClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint point;
	UINT uFlag = 0; // 接收有关点击测试的信息
	GetCursorPos(&point); // 获取屏幕鼠标坐标
	m_Tree.ScreenToClient(&point);
	
	// 点击测试，是否点击了树节点
	HTREEITEM hItem = m_Tree.HitTest(point, &uFlag);
	
	if (NULL != hItem)
	{
		if (uFlag & TVHT_ABOVE)
			MessageBox(_T("L1：TVHT_ABOVE"));
		if (uFlag & TVHT_BELOW)
			MessageBox(_T("L2：TVHT_BELOW"));
		if (uFlag & TVHT_NOWHERE)
			MessageBox(_T("L3：TVHT_NOWHERE"));
		if (uFlag & TVHT_ONITEM)
			MessageBox(_T("L4：TVHT_ONITEM"));
		if (uFlag & TVHT_ONITEMBUTTON)
			MessageBox(_T("L5：TVHT_ONITEMBUTTON"));
		if (uFlag & TVHT_ONITEMICON)
			MessageBox(_T("L6：TVHT_ONITEMICON"));
		if (uFlag & TVHT_ONITEMINDENT)
			MessageBox(_T("L7：TVHT_ONITEMINDENT"));
		if (uFlag & TVHT_ONITEMLABEL)
			MessageBox(_T("L8：TVHT_ONITEMLABEL"));
	
		if (uFlag & TVHT_ONITEMRIGHT)
			MessageBox(_T("L9：TVHT_ONITEMRIGHT"));
	
		if (uFlag & TVHT_ONITEMSTATEICON)
			MessageBox(_T("L10：TVHT_ONITEMSTATEICON"));
	
		if (uFlag & TVHT_TOLEFT)
			MessageBox(_T("L11：TVHT_TOLEFT"));
	
		if (uFlag & TVHT_TORIGHT)
			MessageBox(_T("L12：TVHT_TORIGHT"));
	
	}
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	CPoint ptMouse, ptList;
	UINT uFlag = 0; // 接收有关点击测试的信息
	GetCursorPos(&ptMouse); // 获取屏幕鼠标坐标
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);

	int ListSelected = m_List.HitTest(ptList, &uFlag);
	if (ListSelected < 0) return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPopup = menu.GetSubMenu(0);
	if (pPopup != NULL)
	{
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
	}
}


void CRemoteClientDlg::OnDownloadFile()
{
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);

	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	strFile = GetPath(hSelected) + strFile;//根据选中获取完整路径
	int ret = CClientController::getInstance()->DownloadFile(strFile);
	if (ret != 0)
	{
		MessageBox(_T("下载失败!"));
		TRACE("下载失败ret = %d\r\n", ret);
	}
}

void CRemoteClientDlg::OnRunFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = strPath + strFile;//拼接路径
	TRACE("File Path: %s\r\n", (LPCTSTR)strFile);
	int ret = CClientController::getInstance()->SendCommandPacket(3, (BYTE*)(LPCTSTR)strFile, strFile.GetLength(), true);
	if (ret < 0)
	{
		AfxMessageBox("打开文件命令失败!");
	}
}

//LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)
//{
//	int cmd = wParam >> 1;
//	int ret = 0;
//	switch (cmd)
//	{
//	case 4:
//	{
//		CString strFile = (LPCTSTR)lParam;
//		ret = CClientController::getInstance()->SendCommandPacket(cmd, (BYTE*)(LPCTSTR)strFile, strFile.GetLength(), wParam & 1);
//		break;
//	}
//	case 5://鼠标操作
//	{
//		ret = CClientController::getInstance()->SendCommandPacket(cmd, (BYTE*)lParam,
//			sizeof(MOUSEEV), wParam & 1);
//		TRACE("mouse ret: %d\r\n", ret);
//		break;
//	}
//	case 6:
//	{
//		ret = CClientController::getInstance()->SendCommandPacket(cmd, NULL, 0, wParam & 1);
//		break;
//	}
//	case 7:
//	{
//		ret = CClientController::getInstance()->SendCommandPacket(cmd, NULL, 0, wParam & 1);
//		break;
//	}
//	case 8:
//	{
//		ret = CClientController::getInstance()->SendCommandPacket(cmd, NULL, 0, wParam & 1);
//		break;
//	}
//	default:
//		ret = -1;
//		break;
//	}
//	
//
//	return ret;
//}

void CRemoteClientDlg::OnDeleteFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = strPath + strFile;//拼接路径
	TRACE("File Path: %s\r\n", (LPCTSTR)strFile);
	int ret = CClientController::getInstance()->SendCommandPacket(9, (BYTE*)(LPCTSTR)strFile, strFile.GetLength(), true);
	if (ret < 0)
	{
		AfxMessageBox("删除文件命令失败!");
	}
	LoadFileCurrent();
}

void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	CClientController::getInstance()->StartWatchScreen();
	//m_isClosed = false;
	//CWatchDialog dlg(this);
	//HANDLE hThread = (HANDLE)_beginthread(CRemoteClientDlg::threadEntryForWatchData, 0, this);
	////GetDlgItem(IDC_BTN_START_WATCH)->EnableWindow(FALSE);
	//dlg.DoModal();
	//m_isClosed = true;
	//WaitForSingleObject(hThread, 500);
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}


void CRemoteClientDlg::OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	UpdateData(TRUE);
	CClientController::getInstance()->UpdateAddress(m_server_address, atoi((LPCTSTR)m_port));
}


void CRemoteClientDlg::OnEnChangeEditPort()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	UpdateData(TRUE);
	CClientController::getInstance()->UpdateAddress(m_server_address, atoi((LPCTSTR)m_port));
}
