﻿// RemoteControl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteControl.h"
#include "ServerSocket.h"
#include <direct.h>
#include <io.h>
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

void Dump(BYTE* pData, size_t nSize)
{
    std::string strOut;
    for (size_t i = 0; i < nSize; i++)
    {
        char buffer[8] = "";
        if (i > 0 && i % 16 == 0) strOut += "\n";
        snprintf(buffer, sizeof(buffer), "%02X ", pData[i] & 0xFF);
        strOut += buffer;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}

typedef struct _FILE_INFO
{
    _FILE_INFO()
    {
        IsInvalid = FALSE;
        IsDirectory = -1;
        HasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    }

    BOOL IsInvalid;//是否有效, 0-否 1-是
    BOOL IsDirectory;//是否为目录, 0-否 1-是
    BOOL HasNext;   //是否还有后续
    char szFileName[256];//文件名

} FILEINFO, *PFILEINFO;

//获取磁盘信息
int MakeDriveInfo()//1->A  2->B  3->C ... 26->Z
{
    std::string result;
    for (int i = 1; i <= 26; i++)
    {
        if (_chdrive(i) == 0)
        {
            if (result.size() > 0) result += ',';
            result += 'A' + i - 1;
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());//打包数据
    Dump((BYTE*)pack.Data(), pack.Size());
    //CServerSocket::getInstance()->Send(CPacket(1, (BYTE*)result.c_str(), result.size()));
    //CServerSocket::getInstance()->Send(pack);
    return 0;
}

//获取磁盘分区目录
int MakeDirectoryInfo()
{
    std::string strPath;
    //std::list<FILEINFO> lstFileInfo;
    bool flag = CServerSocket::getInstance()->GetFilePath(strPath);
    if (!flag)
    {
        OutputDebugString(_T("命令解析错误！当前命令不是获取文件列表。"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0)
    {
        FILEINFO finfo;
        finfo.IsInvalid = TRUE;
        finfo.IsDirectory = TRUE;
        finfo.HasNext = FALSE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
        //lstFileInfo.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限访问当前目录。"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1)// * 匹配所有文件
    {
		OutputDebugString(_T("没有找到任何文件。"));
		return -3;
    }
    do 
    {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		//lstFileInfo.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
    } while (!_findnext(hfind, &fdata));
    //发送信息到控制端
    FILEINFO finfo;
    finfo.HasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	CServerSocket::getInstance()->Send(pack);

    return 0;
}

int RunFile()
{
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

    //封包发包
	CPacket pack(3, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
    return 0;
}

//#pragma warning(disable:4996) //禁用所有C4996错误
int DownloadFile()
{
	std::string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
    long long data = 0;
    //FILE* pFile = fopen(strPath.c_str(), "rb");
    FILE* pFile = NULL;
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
	//if (pFile == NULL)
	//{
	//	CPacket pack(4, (BYTE*)&data, 8);
	//	CServerSocket::getInstance()->Send(pack);
	//	return -1;
	//}
    if (err != 0)
    {
		CPacket pack(4, (BYTE*)&data, 8);
		CServerSocket::getInstance()->Send(pack);
		return -1;
    }
    if (pFile != NULL)
    {
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        fseek(pFile, 0, SEEK_SET);

        char buffer[1024]{};
        size_t rlen = 0;
        do
        {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        fclose(pFile);
	}
	CPacket pack(4, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
    return 0;
}

int MouseEvent()
{
    MOUSEEV mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse))
    {
        DWORD nFlags = 0;
        switch (mouse.nButton)
        {
        case 0://左键
            nFlags = 1;
            break;
		case 1://右键
            nFlags = 2;
			break;
		case 2://中键
            nFlags = 4;
			break;
        case 4://没有按下任何键
            nFlags = 8;
            break;
        default:
            break;
        }

        if (nFlags != 8)
        {
			SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        }

        switch (mouse.nAction)
        {
		case 0://单击
            nFlags |= 0x10;
			break;
		case 1://双击
            nFlags |= 0x20;
			break;
		case 2://按下
            nFlags |= 0x40;
			break;
		case 3://放开
            nFlags |= 0x80;
			break;
        default:
            break;
        }

        switch (nFlags)
        {
        case 0x21://左键双击
            //GetMessageExtraInfo => 获取当前线程的额外信息,系统API,例如按下Ctrl加左键这样的操作
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://左键放开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://右键放开
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://中键按下
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://中键放开
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08://鼠标移动
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        default:
            break;
        }

		//封包发包
		CPacket pack(5, NULL, 0);
		CServerSocket::getInstance()->Send(pack);
    }
    else
    {
        OutputDebugString(_T("获取鼠标操作参数失败!"));
        return -1;
    }

    return 0;
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            //1.进度可控 2.对接方便 3.可行性评估，提早暴露风险
            //WSADATA data;
            //WSAStartup(MAKEWORD(1, 1), &data);//返回值处理
			//CServerSocket* pserver = CServerSocket::getInstance();
			//if (!pserver->InitSocket())
			//{
			//	MessageBox(NULL, _T("网络初始化异常,请检查网络状态! "), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
			//	exit(0);
			//}
			//int count = 0;
			//while (pserver != NULL)
			//{
			//	if (!pserver->AcceptClient())
			//	{
			//		if (count >= 3)
			//		{
			//			MessageBox(NULL, _T("多次无法正常接入用户,程序结束! "), _T("接入用户失败"), MB_OK | MB_ICONERROR);
			//			exit(0);
			//		}
			//		MessageBox(NULL, _T("无法正常接入用户,自动重试中! "), _T("接入用户失败"), MB_OK | MB_ICONERROR);
			//		count++;
			//	}
			//	int ret = pserver->DealCommand();
			//}

            //SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);//以TCP流式传输
            //校验
			//sockaddr_in serv_adr, client_adr;
			//memset(&serv_adr, 0, sizeof(serv_adr)); 
            //serv_adr.sin_family = AF_INET;
            //serv_adr.sin_addr.s_addr = INADDR_ANY;
            //serv_adr.sin_port = htons(9527);
            //绑定
            //bind(serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
            //listen(serv_sock, 1);
            //char buffer[1024];
            //int cli_sz = sizeof(client_adr);
            //SOCKET client = accept(serv_sock, &client, &cli_sz);
            //recv(serv_sock, buffer, sizeof(buffer), 0);
            //send(serv_sock, buffer, sizeof(buffer), 0);
            //closesocket(serv_sock);
            //WSACleanup();
            //设置全局静态变量
            int nCmd = 1;
            switch (nCmd)
            {
            case 1://查看磁盘分区信息
                MakeDriveInfo();
                break;
            case 2://查看分区目录信息
                MakeDirectoryInfo();
                break;
            case 3://打开文件
                RunFile();
                break;
			case 4://下载文件
				DownloadFile();
				break;
			case 5://鼠标事件
				MouseEvent();
				break;
            default:
                break;
            }
            
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
