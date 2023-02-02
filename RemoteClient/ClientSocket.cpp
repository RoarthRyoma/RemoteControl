#include "pch.h"
#include "ClientSocket.h"

bool CClientSocket::InitSocket()
{
	if (m_sock != INVALID_SOCKET)
	{
		CloseSocket();
	}
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == -1)
	{
		return false;
	}
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	TRACE("IP address: %08X nIP %08X \r\n", inet_addr("127.0.0.1"), m_nIP);
	serv_adr.sin_addr.s_addr = htonl(m_nIP);
	serv_adr.sin_port = htons(m_nPort);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE)
	{
		AfxMessageBox("指定的IP地址不存在！");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1)
	{
		AfxMessageBox(_T("连接失败！"));
		TRACE("连接失败: %d %s \r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
		return false;
	}
	TRACE("socket init done!\r\n");
	return true;
}


bool CClientSocket::SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed/*=true*/, WPARAM wParam/*=0*/)
{
	if (m_hThread == INVALID_HANDLE_VALUE)
	{
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
	}
	UINT nMode = isAutoClosed ? CSM_AUTOCLOSE : 0;
	std::string strOut;
	pack.Data(strOut);
	return PostThreadMessage(m_nThreadID, WM_SEND_PACK, (WPARAM)new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam), (LPARAM)hWnd);
}


//bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& listPacket, bool isAutoClosed)
//{
//	if (m_sock == INVALID_SOCKET && m_hThread == INVALID_HANDLE_VALUE)
//	{
//		//if (!InitSocket()) return false;
//		m_hThread = (HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
//		TRACE("start threadEntry\r\n");
//	}
//	m_lock.lock();
//	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, listPacket));
//	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
//	TRACE("cmd:%d event:%08X thread_id:%d\r\n", pack.sCmd, pack.hEvent, GetCurrentThreadId());
//	m_listSend.push_back(pack);
//	m_lock.unlock();
//	WaitForSingleObject(pack.hEvent, INFINITE);
//	std::map<HANDLE, std::list<CPacket>&>::iterator it;
//	it = m_mapAck.find(pack.hEvent);
//	if (it != m_mapAck.end())
//	{
//		//std::list<CPacket>::iterator i;
//		//for (i = it->second.begin(); i != it->second.end(); i++)
//		//{
//		//	listPacket.push_back(*i);
//		//}
//		m_lock.lock();
//		m_mapAck.erase(it);
//		m_lock.unlock();
//		return true;
//	}
//	return false;
//}

CClientSocket::CClientSocket() : m_nIP(INADDR_ANY), m_nPort(0), m_sock(INVALID_SOCKET), m_bAutoClose(true), m_hThread(INVALID_HANDLE_VALUE)
{
	if (InitSockEnv() == FALSE)
	{
		MessageBox(NULL, _T("无法初始化Socket环境，请检查网络设置！"), _T("初始化错误!"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	//m_sock = socket(PF_INET, SOCK_STREAM, 0);
	m_buffer.resize(BUFFER_SIZE);
	memset(m_buffer.data(), 0, BUFFER_SIZE);
	struct
	{
		UINT message;
		MSGFUNC func;
	}funcs[] =
	{
		{WM_SEND_PACK,&CClientSocket::SendPack},
		{0,NULL}
	};
	for (int i = 0; funcs[i].message != 0; i++)
	{
		if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(funcs[i].message, funcs[i].func)).second == false)
		{
			TRACE("插入失败，消息值: %d 函数值: %08X 序号: %d\r\n", funcs[i].message, funcs[i].func, i);
		}
	}
}

CClientSocket::CClientSocket(const CClientSocket& s)
{
	m_sock = s.m_sock;
	m_nIP = s.m_nIP;
	m_nPort = s.m_nPort;
	m_bAutoClose = s.m_bAutoClose;
	m_hThread = s.m_hThread;
	m_nThreadID = s.m_nThreadID;
	std::map<UINT, MSGFUNC>::const_iterator it = s.m_mapFunc.begin();
	for (; it != s.m_mapFunc.end(); it++)
	{
		m_mapFunc.insert(std::pair<UINT, MSGFUNC>(it->first, it->second));
	}
}

//void CClientSocket::threadFunc()
//{
//	std::string strBuffer;
//	strBuffer.resize(BUFFER_SIZE);
//	char* pBuffer = (char*)strBuffer.c_str();
//	int index = 0;
//	InitSocket();
//	while (m_sock != INVALID_SOCKET)
//	{
//		if (m_listSend.size() > 0)
//		{
//			TRACE("listSend size: %d\r\n", m_listSend.size());
//			m_lock.lock();
//			CPacket& head = m_listSend.front();
//			m_lock.unlock();
//			if (Send(head) == false)
//			{
//				TRACE("发送失败!\r\n");
//				break;
//				//continue;
//			}
//			
//			std::map<HANDLE, std::list<CPacket>&>::iterator it;
//			it = m_mapAck.find(head.hEvent);
//			if (it != m_mapAck.end())
//			{
//				auto bIt = m_mapAutoClosed.find(head.hEvent);
//				do
//				{
//					int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
//					//if (!(length <= 0 && index <= 0))
//					if (length > 0 || (index > 0))
//					{
//						index += length;
//						size_t size = (size_t)index;
//						CPacket pack((BYTE*)pBuffer, size);
//						if (size > 0)
//						{//TODO:文件夹信息获取，文件信息获取可能产生问题
//							pack.hEvent = head.hEvent;
//							it->second.push_back(pack);
//							memmove(pBuffer, pBuffer + size, index - size);
//							index -= size;
//							if (bIt->second)
//							{
//								SetEvent(head.hEvent);
//								break;
//							}
//						}
//					}
//					else if (length <= 0 && index <= 0)
//					{
//						CloseSocket();
//						SetEvent(head.hEvent);//等到服务器关闭命令后在通知事件完成
//						if (bIt != m_mapAutoClosed.end())
//						{
//							TRACE("SetEvent %d %d\r\n", head.sCmd, bIt->second);
//						}
//						else
//						{
//							TRACE("异常的情况，没有对应的pair\r\n");
//						}
//						break;
//					}
//				} while (!bIt->second);
//			}
//			m_lock.lock();
//			m_listSend.pop_front();
//			m_mapAutoClosed.erase(head.hEvent);
//			m_lock.unlock();
//			if(!InitSocket()) InitSocket();
//		}
//	}
//	CloseSocket();
//}

void CClientSocket::threadFunc2()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (m_mapFunc.find(msg.message) != m_mapFunc.end())
		{
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}

unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* that = (CClientSocket*)arg;
	//that->threadFunc();
	that->threadFunc2();
	_endthreadex(0);
	return 0;
}

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{//定义一个消息的数据结构(数据和数据长度, 模式,回调消息的数据结构(HWND))
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	if (InitSocket())
	{
		int ret = send(m_sock, (char*)data.strData.c_str(), (int)data.strData.size(), 0);
		if (ret > 0)
		{
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();
			while (m_sock != INVALID_SOCKET)
			{
				int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
				if ((length > 0) || (index > 0))
				{
					index += (size_t)length;
					size_t nLen = index;
					CPacket pack((BYTE*)pBuffer, nLen);
					if (nLen > 0)
					{
						TRACE("ack pack %d to hWnd %08X %d %d\r\n", pack.sCmd, hWnd, index, nLen);
						TRACE("%04X\r\n", *(WORD*)(pBuffer + nLen));
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack), data.wParam);
						if (data.nMode & CSM_AUTOCLOSE)
						{
							CloseSocket();
							return;
						}
						index -= nLen;
						memmove(pBuffer, pBuffer + nLen, index);
					}
				}
				else
				{
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, 1);
				}
			}
		}
		else
		{
			CloseSocket();
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else
	{
		//错误处理
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -2);
	}
}

//CClientSocket server;
CClientSocket* CClientSocket::m_instance = NULL;

CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();

std::string GetErrorInfo(int wsaErrCode)
{
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	ret = (char*)lpMsgBuf;

	LocalFree(lpMsgBuf);
	return ret;
}