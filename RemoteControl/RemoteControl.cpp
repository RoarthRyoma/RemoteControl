// RemoteControl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteControl.h"
#include "ServerSocket.h";

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

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
                int ret = pserver->DealCommand();
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
