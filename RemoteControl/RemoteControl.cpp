// RemoteControl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteControl.h"
#include "ServerSocket.h"
//#include <direct.h>
//#include <io.h>
//#include <list>
//#include <atlimage.h>
//#include "LockDialog.h"
#include "EdoyunTool.h"
#include "Command.h"
#include <conio.h>
#include "Queue.h"
#include <MSWSock.h>
#include "Server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define INVOKE_PATH _T("C:\\Windows\\SysWOW64\\RemoteServer.exe")
#define INVOKE_PATH _T("C:\\Users\\Administrator\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteServer.exe")

// 唯一的应用程序对象
CWinApp theApp;

using namespace std;

//业务和通用
bool ChooseAutoInvoke(const CString& strPath)
{
    if (PathFileExists(strPath))
    {
        return true;
    }
	CString strInfo = _T("该程序只允许用于合法的用途！\n");
	strInfo += _T("继续运行该程序，将使得这台机器处于被监控状态！\n");
	strInfo += _T("如果您不希望这样，请按下“取消”按钮，退出程序！\n");
	strInfo += _T("按下”是“按钮，该程序将被复制到您的机器上，并随系统启动而自动运行！\n");
	strInfo += _T("按下”否“按钮，程序只运行一次，不会在系统内留下任何东西！\n");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
	if (ret == IDYES)
	{
        //WriteRegisterTable();
		if (CEdoyunTool::WriteStartupDir(strPath))
		{
			MessageBox(NULL, _T("复制文件失败，是否权限不足?\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		return true;
	}
	else if (ret == IDCANCEL)
	{
		return false;
	}
	return true;
}

//class COverlapped
//{
//public:
//	OVERLAPPED m_overlapped;
//	DWORD m_operator;
//	char m_buffer[4096];
//	COverlapped()
//	{
//		m_operator = 0;
//		memset(&m_overlapped, 0, sizeof(m_overlapped));
//		memset(m_buffer, 0, sizeof(m_buffer));
//	}
//};

void Iocp()
{
	CServer server;
	server.StartService();
	getchar();
}

int main()
{
	if (!CEdoyunTool::Init()) return 1;

	Iocp();

	//if (CEdoyunTool::IsAdmin())
	//{
	//	if (!CEdoyunTool::Init()) return 1;
	//	if (ChooseAutoInvoke(INVOKE_PATH))
	//	{
	//		CCommand cmd;
	//		int ret = CServerSocket::getInstance()->Run(CCommand::RunCommand, &cmd);
	//		switch (ret)
	//		{
	//		case -1:
	//			MessageBox(NULL, _T("网络初始化异常,请检查网络状态! "), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
	//			break;
	//		case -2:
	//			MessageBox(NULL, _T("多次无法正常接入用户,程序结束! "), _T("接入用户失败"), MB_OK | MB_ICONERROR);
	//			break;
	//		}
	//	}
	//}
	//else
	//{
	//	if (CEdoyunTool::RunAsAdmin() == false)
	//	{
	//		CEdoyunTool::ShowError();
	//		return 1;
	//	}
	//}
    return 0;
}