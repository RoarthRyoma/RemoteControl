﻿// RemoteControl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteControl.h"
#include "ServerSocket.h"
#include <direct.h>
#include <io.h>
#include <list>
#include <atlimage.h>
#include "LockDialog.h"


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
    CServerSocket::getInstance()->Send(pack);
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
        //finfo.IsInvalid = TRUE;
        //finfo.IsDirectory = TRUE;
        finfo.HasNext = FALSE;
        //memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
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
		FILEINFO finfo;
		finfo.HasNext = FALSE;
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
		return -3;
    }
    int COUNT = 0;
    do 
    {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		//lstFileInfo.push_back(finfo);
        TRACE("file info: %s \r\n", finfo.szFileName);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
        COUNT++;
    } while (!_findnext(hfind, &fdata));
    //发送信息到控制端
    FILEINFO finfo;
    finfo.HasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	CServerSocket::getInstance()->Send(pack);
    TRACE("server COUNT: %d\r\n", COUNT);
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
        CServerSocket::getInstance()->Send(head);
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
        TRACE("mouse event : %08X, x=%d y=%d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
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

int SendScreen()
{
    CImage screen;//GDI  图形设备接口(Graphics Device Interface)
    HDC hScreen = ::GetDC(NULL);
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == NULL) return -1;
    IStream* pStream = NULL;
    HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE/*自己管理内存时使用FALSE*/, &pStream);
    if (hRet == S_OK)
    {
        screen.Save(pStream, Gdiplus::ImageFormatJPEG);
        LARGE_INTEGER bg = { 0 };
        pStream->Seek(bg, STREAM_SEEK_SET, NULL);
        PBYTE pData = (PBYTE)GlobalLock(hMem);
        SIZE_T nSize = GlobalSize(hMem);
		CPacket pack(6, pData, nSize);
		CServerSocket::getInstance()->Send(pack);
        GlobalUnlock(hMem);
    }
    pStream->Release();
    GlobalFree(hMem);
    //screen.Save(_T("test2022.jpg"), Gdiplus::ImageFormatJPEG);

	//ULONGLONG tick = GetTickCount64();
	//screen.Save(_T("test2022.png"), Gdiplus::ImageFormatPNG);
	//TRACE("png: %d\r\n", GetTickCount64() - tick);
	//tick = GetTickCount64();
	//screen.Save(_T("test2022.jpg"), Gdiplus::ImageFormatJPEG);
	//TRACE("jpeg: %d\r\n", GetTickCount64() - tick);
    screen.ReleaseDC();
    return 0;
}

CLockDialog dlg;
unsigned threadid = 0;

unsigned _stdcall ThreadLockDlg(void* arg)
{
    TRACE("%s(%d)%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());

	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	//遮蔽后台窗口
	CRect rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//w1
	rect.bottom = (LONG)(GetSystemMetrics(SM_CYFULLSCREEN) * 1.1);//h1
	TRACE("right = %d bottom = %d\r\n", rect.right, rect.bottom);
	dlg.MoveWindow(rect);
    CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
    if (pText)
    {
        CRect rtText;
        pText->GetWindowRect(rtText);
        int nWidth = rtText.Width();//w0;
        int nHeight = rtText.Height();//h0
        int x = (rect.right - nWidth) / 2;
        int y = (rect.bottom - nHeight) / 2;
        pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
    }
	//窗口置顶
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	//隐藏鼠标位置
	ShowCursor(false);
	//隐藏任务栏
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
	//限制鼠标活动范围
	dlg.GetWindowRect(rect);
	ClipCursor(rect);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_KEYDOWN)
		{
			TRACE("msg:%08X lparam:%08X wparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
			if (msg.wParam == 0x1B)//按ESC退出
			{
				break;
			}
		}
	}
    ClipCursor(NULL);
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
	ShowCursor(true);
	dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}

int LockMachine()
{
    if ((dlg.m_hWnd == NULL)||(dlg.m_hWnd == INVALID_HANDLE_VALUE))
    {
        //_beginthread(ThreadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, ThreadLockDlg, NULL, 0, &threadid);
		TRACE("threadid - %d\r\n", threadid);
    }
	CPacket pack(7, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
    return 0;
}

int UnlockMachine()
{
    //dlg.SendMessage(WM_KEYDOWN, 0x1B, 0x00010001);
    //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1B, 0x00010001);
    PostThreadMessage(threadid, WM_KEYDOWN, 0x1B, 0);
	CPacket pack(8, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
    return 0;
}

int TestConnect()
{
    CPacket pack(1981, NULL, 0);
    bool ret = CServerSocket::getInstance()->Send(pack);
    TRACE(" test Send ret: %d\r\n", ret);
    return 0;
}

int DeleteLocalFile()
{
	std::string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
    TCHAR sPath[MAX_PATH] = _T("");

	/*//中文容易乱码
    mbstowcs(sPath, strPath.c_str(), strPath.size());*/
    //WindowsAPI处理宽字节和多字节的转码
    MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));
	DeleteFile(sPath);
	CPacket pack(9, NULL, 0);
	bool ret = CServerSocket::getInstance()->Send(pack);
	TRACE(" delete file Send ret: %d\r\n", ret);
	return 0;
	/*//不报C4996安全错误的写法
    size_t converted = strPath.size() + 1;
	errno_t err = mbstowcs_s(&converted, sPath, converted, strPath.c_str(), _TRUNCATE);
	if (err == 0)
	{
		DeleteFile(sPath);
		CPacket pack(9, NULL, 0);
		bool ret = CServerSocket::getInstance()->Send(pack);
		TRACE(" delete file Send ret: %d\r\n", ret);
		return 0;
	}
	else
	{
		TRACE("delete file failed\r\n");
		return -1;
	}*/
}

int ExecuteCommand(int nCmd)
{
    int ret = 0;
	switch (nCmd)
	{
	case 1://查看磁盘分区信息
        ret = MakeDriveInfo();
		break;
	case 2://查看分区目录信息
        ret = MakeDirectoryInfo();
		break;
	case 3://打开文件
        ret = RunFile();
		break;
	case 4://下载文件
        ret = DownloadFile();
		break;
	case 5://鼠标事件
        ret = MouseEvent();
		break;
	case 6://发送屏幕内容 => 发送屏幕截图
        ret = SendScreen();
		break;
	case 7://锁机
        ret = LockMachine();
		break;
	case 8://解锁
        ret = UnlockMachine();
		break;
    case 9://删除文件
        ret = DeleteLocalFile();
		break;
    case 1981:
        ret = TestConnect();
        break;
	default:
		break;
	}
    return ret;
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
			CServerSocket* pserver = CServerSocket::getInstance();
			if (!pserver->InitSocket())
			{
				MessageBox(NULL, _T("网络初始化异常,请检查网络状态! "), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
				exit(0);
			}
			int count = 0;
			while (pserver != NULL)
			{
				if (!pserver->AcceptClient())
				{
					if (count >= 3)
					{
						MessageBox(NULL, _T("多次无法正常接入用户,程序结束! "), _T("接入用户失败"), MB_OK | MB_ICONERROR);
						exit(0);
					}
					MessageBox(NULL, _T("无法正常接入用户,自动重试中! "), _T("接入用户失败"), MB_OK | MB_ICONERROR);
					count++;
				}
                TRACE("Accept return true\r\n");
				int ret = pserver->DealCommand();
                TRACE("DealCommand ret: %d\r\n", ret);
                if (ret > 0)
                {
                    ret = ExecuteCommand(ret);
                    if (ret != 0)
                    {
                        TRACE("执行命令失败:%d ret=%d\r\n", pserver->GetPacket().sCmd, ret);
                    }
                    pserver->CloseClient();
                    TRACE("Command has done!\r\n");
                }
			}

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
            //int nCmd = 7;
            //switch (nCmd)
            //{
            //case 1://查看磁盘分区信息
            //  MakeDriveInfo();
            //  break;
            //case 2://查看分区目录信息
            //  MakeDirectoryInfo();
            //  break;
            //case 3://打开文件
            //  RunFile();
            //  break;
			//case 4://下载文件
			//	DownloadFile();
			//	break;
			//case 5://鼠标事件
			//	MouseEvent();
			//	break;
            //case 6://发送屏幕内容 => 发送屏幕截图
            //  SendScreen();
            //  break;
			//case 7://鼠标事件
			//	LockMachine();
            //  //Sleep(500);
			//	//LockMachine();
			//	break;
			//case 8://鼠标事件
            //  UnlockMachine();
			//	break;
            //default:
            //  break;
            //}
            //Sleep(2000);
            //UnlockMachine();
			//while ((dlg.m_hWnd != NULL) && (dlg.m_hWnd != INVALID_HANDLE_VALUE))
			//	Sleep(100);
			//TRACE("m_hWnd = %08X\r\n", dlg.m_hWnd);


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
