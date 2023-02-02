#include "pch.h"
#include "ClientController.h"

std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController*  CClientController::m_instance = NULL;
CClientController::CHelper CClientController::m_helper;

CClientController* CClientController::getInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new CClientController();
		TRACE("CClientController size is %d\r\n", sizeof(*m_instance));
		struct 
		{
			UINT nMsg; MSGFUNC func;
		} MsgFuncs[] = {
			//{WM_SEND_PACK, &CClientController::OnSendPack},
			//{WM_SEND_DATA, &CClientController::OnSendData},
			{WM_SHOW_STATUS, &CClientController::OnShowStatus},
			{WM_SHOW_WATCH, &CClientController::OnShowWatch},
			{(UINT)-1, NULL}
		};
		for (int i = 0; MsgFuncs[i].func != NULL; i++)
		{
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFuncs[i].nMsg, MsgFuncs[i].func));
		}
	}
	return m_instance;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, CClientController::threadEntry, this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS, &m_remoteDlg);
	return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

LRESULT CClientController::SendMessage(MSG msg)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL) return -2;
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)&hEvent);
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);
	return info.result;
}

bool CClientController::SendCommandPacket(HWND hWnd, int nCmd, BYTE* pData /*= NULL*/, size_t nLength /*= 0*/, bool bAutoClose /*= true*/, WPARAM wParam /*= 0*/)
{
	TRACE("cmd:%d %s start %lld \r\n", nCmd, __FUNCTION__, GetTickCount64());
	return CClientSocket::getInstance()->SendPacket(hWnd, CPacket(nCmd, pData, nLength), bAutoClose, wParam);
}

int CClientController::DownloadFile(CString strPath)
{
	CFileDialog dlg(FALSE, "*", strPath,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		NULL, &m_remoteDlg);
	if (dlg.DoModal() == IDOK)
	{
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		FILE* pFile = NULL;
		errno_t err = fopen_s(&pFile, m_strLocal, "wb+");
		if (err != 0)
		{
			AfxMessageBox(_T("本地没有权限保存该文件，或者文件无法创建！"));
			return -1;
		}
		SendCommandPacket(m_remoteDlg, 4, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), false, (WPARAM)pFile);
		/*             添加线程函数          */
		//m_hThreadDownload = (HANDLE)_beginthread(&CClientController::threadDownloadEntry, 0, this);
		//Sleep(50);//休眠50毫秒启用线程
		//if (WaitForSingleObject(m_hThreadDownload, 0) != WAIT_TIMEOUT)
		//{
		//	return -1;
		//}
		m_remoteDlg.BeginWaitCursor();
		m_statusDlg.m_info.SetWindowText(_T("命令正在执行中..."));
		m_statusDlg.CenterWindow(&m_remoteDlg);
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.SetActiveWindow();
	}
	return 0;
}

void CClientController::DownloadEnd()
{
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成！"), _T("完成"));
}

void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	//m_watchDlg.SetParent(&m_remoteDlg);
	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadWatchScreenEntry, 0, this);
	m_watchDlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);
}

void CClientController::threadWatchScreen()
{
	Sleep(50);
	while (!m_isClosed)
	{
		if (!m_watchDlg.isFull())//数据更新到缓存
		{
			std::list<CPacket> listPacket;
			int ret = SendCommandPacket(m_watchDlg.GetSafeHwnd(), 6, NULL, 0, true);
			//todo: 添加消息响应函数 WM_SEND_PACK_ACK
			//todo: 控制发送频率
			if (ret == 6)
			{
				int res = CEdoyunTool::Byte2Image(m_watchDlg.GetImage(), listPacket.front().strData);
				if (res == 0)
				{
					m_watchDlg.SetImageStatus(true);
				}
				else
				{
					TRACE("获取图片失败！ret=%d \r\n", ret);
				}
			}
			else
			{
				Sleep(1);
			}
		}
		else
		{
			Sleep(1);
		}
	}
}

void CClientController::threadWatchScreenEntry(void* arg)
{
	CClientController* that = (CClientController*)arg;
	that->threadWatchScreen();
	_endthreadex(0);

}

void CClientController::threadDownloadFile()
{
	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, m_strLocal, "wb+");
	if (err != 0)
	{
		AfxMessageBox(_T("没有权限保存该文件或文件无法创建!"));
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	do 
	{
		int ret = SendCommandPacket(m_remoteDlg, 4, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), false, (WPARAM)pFile);
		if (ret < 0)
		{
			AfxMessageBox("下载文件命令执行失败!");
			TRACE("执行download ret: %d\r\n", ret);
			break;
		}
		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
		if (nLength == 0)
		{
			AfxMessageBox("文件长度为0或无法读取文件");
			//return;
			break;
		}
		long long nCount = 0;
		while (nCount < nLength)
		{
			ret = pClient->DealCommand();
			if (ret < 0)
			{
				AfxMessageBox("传输失败!");
				TRACE("传输失败 ret:%d", ret);
				break;
			}
			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
			nCount += pClient->GetPacket().strData.size();
		}
	} while (false);
	fclose(pFile);
	pClient->CloseSocket();

	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成!!!"), _T("完成"));
}

void CClientController::threadDownloadEntry(void* arg)
{
	CClientController* that = (CClientController*)arg;
	that->threadDownloadFile();
	_endthreadex(0);
}

void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(pmsg->msg.message);
			if (it != m_mapFunc.end())
			{
				pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else
			{
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else
		{
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
			{
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}
		
	}
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* that = (CClientController*)arg;
	that->threadFunc();
	_endthreadex(0);
	return 0;
}

//LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	CPacket* pPacket = (CPacket*)wParam;
//	return pClient->Send(*pPacket);
//}

//LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	char* pBuffer = (char*)wParam;
//	return pClient->Send(pBuffer, (int)lParam);
//}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}
