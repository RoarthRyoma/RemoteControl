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

#define IOCP_LIST_EMPTY 0
#define IOCP_LIST_PUSH 1
#define IOCP_LIST_POP 2

enum
{
	IocpListEmpty,
	IocpListPush,
	IocpListPop
};

typedef struct IocpParam
{
	size_t nOperator;		//操作
	string strData;			//数据
	_beginthread_proc_type cbFunc;//回调
	IocpParam(int op, const char* sData, _beginthread_proc_type cb = NULL)
	{
		nOperator = op;
		strData = sData;
		cbFunc = cb;
	}
	IocpParam()
	{
		nOperator = -1;
		cbFunc = nullptr;
	}
}IOCP_PARAM;

void threadMain(HANDLE hIOCP)
{
	list<string> lstString;
	DWORD dwTransferred = 0;
	ULONG_PTR CompletionKey = 0;
	OVERLAPPED* pOverlapped = NULL;
	int charge1 = 0, charge2 = 0;
	while (GetQueuedCompletionStatus(hIOCP, &dwTransferred, &CompletionKey, &pOverlapped, INFINITE))
	{
		if (dwTransferred == 0 && CompletionKey == NULL)
		{
			printf_s("thread is prepare to exit!\r\n");
			break;
		}
		IOCP_PARAM* pParam = (IOCP_PARAM*)CompletionKey;
		if (pParam->nOperator == IocpListPush)
		{
			lstString.push_back(pParam->strData);
			charge1++;
		}
		else if (pParam->nOperator == IocpListPop)
		{
			printf_s("%p -- %d \r\n", pParam->cbFunc, lstString.size());
			string* pStr = NULL;
			if (lstString.size() > 0)
			{
				pStr = new string(lstString.front());
				lstString.pop_front();
			}
			if (pParam->cbFunc)
			{
				pParam->cbFunc(pStr);
			}
			charge2++;
		}
		else if (pParam->nOperator == IocpListEmpty)
		{
			lstString.clear();
		}
		delete pParam;
	}
	printf_s("charge1 = %d,  charge2 = %d\r\n", charge1, charge2);
}

void threadQueueEntry(HANDLE hIOCP)
{
	threadMain(hIOCP);
	_endthread();//代码到此为止,会导致本地对象无法调用析构,从而导致内存泄漏
}

void func(void* arg)
{
	string* pstr = (string*)arg;
	if (pstr != NULL)
	{
		printf_s("pop from list: %s\r\n", pstr->c_str());
		delete pstr;
	}
	else
	{
		printf_s("list is empty, no data!\r\n");
	}
	
}

void test(CQueue<string>& lstString, list<string>& lstData)
{
	//printf_s("press any key to exit!\r\n");
	ULONGLONG tick = GetTickCount64();
	ULONGLONG tick0 = GetTickCount64();
	ULONGLONG total = GetTickCount64();
	//while (_kbhit() == 0)//完成端口 把请求和实现分离了
	while (GetTickCount64() - total <= 1000)
	{
		//if (GetTickCount64() - tick0 > 5)
		//{
			lstString.PushBack("Hello world!");
			tick0 = GetTickCount64();
		//}
	}
	size_t count = lstString.Size();
	printf_s("exit done! lstString size = %d\r\n", count);
	total = GetTickCount64();
	while(GetTickCount64() - total <= 1000)
	{
		//if (GetTickCount64() - tick > 5)
		//{
			string str;
			lstString.PopFront(str);
			tick = GetTickCount64();
		//}
		//Sleep(1);
	}
	printf_s("exit done! lstString size = %d\r\n", count - lstString.Size());
	lstString.Clear();
	total = GetTickCount64();
	while (GetTickCount64() - total <= 1000)
	{
		lstData.push_back("Hello world!");
	}
	count = lstData.size();
	printf_s("exit done! lstData size = %d\r\n", count);
	total = GetTickCount64();
	while (GetTickCount64() - total <= 250)
	{
		if(lstData.size() > 0) lstData.pop_front();
	}
	printf_s("exit done! lstData size = %d\r\n", (count - lstData.size()) * 4);
	//lstData.clear();
}

int main()
{
	if (!CEdoyunTool::Init()) return 1;
	CQueue<string> lstString;
	list<string> lstData;
	for (int i = 0; i < 10; i++)
	{
		test(lstString, lstData);
	}

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