#pragma once
#include "ClientSocket.h"
#include "RemoteClientDlg.h"
#include "WatchDialog.h"
#include "StatusDlg.h"
#include "Resource.h"
#include "EdoyunTool.h"
#include <map>

//#define WM_SEND_PACK (WM_USER+1)	//发送包数据
//#define WM_SEND_DATA (WM_USER+2)	//发送数据
#define WM_SHOW_STATUS (WM_USER+3)	//展示状态
#define WM_SHOW_WATCH (WM_USER+4)	//远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000)	//自定义消息处理

class CClientController
{
public:
	//获取全局唯一对象
	static CClientController* getInstance();
	//初始化控制器
	int InitController();
	//启动
	int Invoke(CWnd*& pMainWnd);
	//发送消息
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
	//int SendPacket(const CPacket& pack)
	//{
	//	CClientSocket* pClient = CClientSocket::getInstance();
	//	if (!pClient->InitSocket()) return false;
	//	pClient->Send(pack);
	//}

	CPacket& GetPacket()
	{
		return CClientSocket::getInstance()->GetPacket();
	}

	/// <summary>
	/// 1.查看磁盘分区信息
	///	2.查看分区目录信息
	///	3.打开文件
	///	4.下载文件
	///	5.鼠标事件
	///	6.发送屏幕内容 = > 发送屏幕截图
	///	7.锁机
	///	8.解锁
	/// </summary>
	/// <param name="hWnd">数据包收到后需要应答的窗口</param>
	/// <param name="nCmd">命令</param>
	/// <param name="pData">数据</param>
	/// <param name="nLength">数据长度</param>
	/// <param name="bAutoClose"></param>
	/// <param name="pListPacket">应答包列表</param> //移除,现在转为消息处理
	/// <returns>状态,true成功,false失败</returns>
	bool SendCommandPacket(HWND hWnd, int nCmd, BYTE* pData = NULL, size_t nLength = 0, bool bAutoClose = true/*, std::list<CPacket>* pListPacket = nullptr*/);
	int GetImage(CImage& image)
	{
		auto pClient = CClientSocket::getInstance();
		return CEdoyunTool::Byte2Image(image, pClient->GetPacket().strData);
	}
	int DownloadFile(CString strPath);
	void StartWatchScreen();
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
			TRACE("CClientController has released.\r\n");
		}
	}

	//LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
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
	bool m_isClosed;//监视线程是否关闭
	//下载文件的远程路径
	CString m_strRemote;
	//下载文件的本地保存路径
	CString m_strLocal;
	unsigned m_nThreadID;
	static CClientController* m_instance;
	class CHelper
	{
	public:
		CHelper(){}
		~CHelper()
		{
			CClientController::ReleaseInstance();
		}
	};
	static CHelper m_helper;
};

