#pragma once
#include "ClientSocket.h"
#include "RemoteClientDlg.h"
#include "WatchDialog.h"
#include "StatusDlg.h"
#include "Resource.h"
#include "EdoyunTool.h"
#include <map>

#define WM_SEND_PACK (WM_USER+1)	//���Ͱ�����
#define WM_SEND_DATA (WM_USER+2)	//��������
#define WM_SHOW_STATUS (WM_USER+3)	//չʾ״̬
#define WM_SHOW_WATCH (WM_USER+4)	//Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000)	//�Զ�����Ϣ����

class CClientController
{
public:
	//��ȡȫ��Ψһ����
	static CClientController* getInstance();
	//��ʼ��������
	int InitController();
	//����
	int Invoke(CWnd*& pMainWnd);
	//������Ϣ
	LRESULT SendMessage(MSG msg);
	void UpdateAddress(int nIP, int nPort)
	{
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand()
	{
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket()
	{
		CClientSocket::getInstance()->CloseSocket();
	}
	int SendPacket(const CPacket& pack)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		if (!pClient->InitSocket()) return false;
		pClient->Send(pack);
	}

	CPacket& GetPacket()
	{
		return CClientSocket::getInstance()->GetPacket();
	}

	/// <summary>
	/// 1.�鿴���̷�����Ϣ
	///	2.�鿴����Ŀ¼��Ϣ
	///	3.���ļ�
	///	4.�����ļ�
	///	5.����¼�
	///	6.������Ļ���� = > ������Ļ��ͼ
	///	7.����
	///	8.����
	/// </summary>
	/// <param name="nCmd">����</param>
	/// <param name="pData">����</param>
	/// <param name="nLength">���ݳ���</param>
	/// <returns>��������ֵ</returns>
	int SendCommandPacket(int nCmd, BYTE* pData = NULL, size_t nLength = 0, bool bAutoClose = true)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		if (!pClient->InitSocket()) return false;
		pClient->Send(CPacket(nCmd, pData, nLength));
		int cmd = DealCommand();
		TRACE("ack: %d\r\n", cmd);
		if (bAutoClose)
		{
			CloseSocket();
		}
		return cmd;
	}
	int GetImage(CImage& image)
	{
		auto pClient = CClientSocket::getInstance();
		return CEdoyunTool::Byte2Image(image, pClient->GetPacket().strData);
	}
	int DownloadFile(CString strPath)
	{
		CFileDialog dlg(FALSE, "*", strPath,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			NULL, &m_remoteDlg);
		if (dlg.DoModal() == IDOK)
		{
			m_strRemote = strPath;
			m_strLocal = dlg.GetPathName();

			/*             �����̺߳���          */
			m_hThreadDownload = (HANDLE)_beginthread(&CClientController::threadDownloadEntry, 0, this);
			//Sleep(50);//����50���������߳�
			if (WaitForSingleObject(m_hThreadDownload, 0) != WAIT_TIMEOUT)
			{
				return -1;
			}
			m_remoteDlg.BeginWaitCursor();
			m_statusDlg.m_info.SetWindowText(_T("��������ִ����..."));
			m_statusDlg.CenterWindow(&m_remoteDlg);
			m_statusDlg.ShowWindow(SW_SHOW);
			m_statusDlg.SetActiveWindow();
		}
		return 0;
	}
	void StartWatchScreen()
	{
		m_isClosed = false;
		CWatchDialog dlg(&m_remoteDlg);
		m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadWatchScreenEntry, 0, this);
		dlg.DoModal();
		m_isClosed = true;
		WaitForSingleObject(m_hThreadWatch, 500);
	}
protected:
	void threadWatchScreen();
	static void threadWatchScreenEntry(void* arg);
	void threadDownloadFile();
	static void threadDownloadEntry(void* arg);
	CClientController() : m_statusDlg(&m_remoteDlg), m_watchDlg(&m_remoteDlg)
	{
		m_isClosed = true;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_hThreadDownload = INVALID_HANDLE_VALUE;
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController()
	{
		WaitForSingleObject(m_hThread, 100);
	}

	void threadFunc();

	static unsigned __stdcall threadEntry(void* arg);

	static void ReleaseInstance()
	{
		if (m_instance != nullptr)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}

	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo
	{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m)
		{
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m)
		{
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m)
		{
			if (this != &m)
			{
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	} MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	HANDLE m_hThreadDownload;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//�����߳��Ƿ�ر�
	//�����ļ���Զ��·��
	CString m_strRemote;
	//�����ļ��ı��ر���·��
	CString m_strLocal;
	unsigned m_nThreadID;
	static CClientController* m_instance;
	class CHelper
	{
	public:
		CHelper()
		{
			CClientController::getInstance();
		}
		~CHelper()
		{
			CClientController::ReleaseInstance();
		}
	};
};
