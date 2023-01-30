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
		AfxMessageBox("ָ����IP��ַ�����ڣ�");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1)
	{
		AfxMessageBox(_T("����ʧ�ܣ�"));
		TRACE("����ʧ��: %d %s \r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
		return false;
	}
	TRACE("socket init done!\r\n");
	return true;
}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& listPacket, bool isAutoClosed)
{
	if (m_sock == INVALID_SOCKET && m_hThread == INVALID_HANDLE_VALUE)
	{
		//if (!InitSocket()) return false;
		m_hThread = (HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
		TRACE("start threadEntry\r\n");
	}
	m_lock.lock();
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, listPacket));
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	TRACE("cmd:%d event:%08X thread_id:%d\r\n", pack.sCmd, pack.hEvent, GetCurrentThreadId());
	m_listSend.push_back(pack);
	m_lock.unlock();
	WaitForSingleObject(pack.hEvent, INFINITE);
	std::map<HANDLE, std::list<CPacket>&>::iterator it;
	it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end())
	{
		//std::list<CPacket>::iterator i;
		//for (i = it->second.begin(); i != it->second.end(); i++)
		//{
		//	listPacket.push_back(*i);
		//}
		m_lock.lock();
		m_mapAck.erase(it);
		m_lock.unlock();
		return true;
	}
	return false;
}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	InitSocket();
	while (m_sock != INVALID_SOCKET)
	{
		if (m_listSend.size() > 0)
		{
			TRACE("listSend size: %d\r\n", m_listSend.size());
			m_lock.lock();
			CPacket& head = m_listSend.front();
			m_lock.unlock();
			if (Send(head) == false)
			{
				TRACE("����ʧ��!\r\n");
				break;
				//continue;
			}
			
			std::map<HANDLE, std::list<CPacket>&>::iterator it;
			it = m_mapAck.find(head.hEvent);
			if (it != m_mapAck.end())
			{
				auto bIt = m_mapAutoClosed.find(head.hEvent);
				do
				{
					int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
					//if (!(length <= 0 && index <= 0))
					if (length > 0 || (index > 0))
					{
						index += length;
						size_t size = (size_t)index;
						CPacket pack((BYTE*)pBuffer, size);
						if (size > 0)
						{//TODO:�ļ�����Ϣ��ȡ���ļ���Ϣ��ȡ���ܲ�������
							pack.hEvent = head.hEvent;
							it->second.push_back(pack);
							memmove(pBuffer, pBuffer + size, index - size);
							index -= size;
							if (bIt->second)
							{
								SetEvent(head.hEvent);
								break;
							}
						}
					}
					else if (length <= 0 && index <= 0)
					{
						CloseSocket();
						SetEvent(head.hEvent);//�ȵ��������ر��������֪ͨ�¼����
						if (bIt != m_mapAutoClosed.end())
						{
							TRACE("SetEvent %d %d\r\n", head.sCmd, bIt->second);
						}
						else
						{
							TRACE("�쳣�������û�ж�Ӧ��pair\r\n");
						}
						break;
					}
				} while (!bIt->second);
			}
			m_lock.lock();
			m_listSend.pop_front();
			m_mapAutoClosed.erase(head.hEvent);
			m_lock.unlock();
			if(!InitSocket()) InitSocket();
		}
	}
	CloseSocket();
}

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

void /*unsigned*/ CClientSocket::threadEntry(void* arg)
{
	CClientSocket* that = (CClientSocket*)arg;
	//that->threadFunc();
	that->threadFunc2();
	_endthreadex(0);
	//return 0;
}

void CClientSocket::SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if (InitSocket())
	{
		int ret = send(m_sock, (char*)wParam, (int)lParam, 0);
		if (ret > 0)
		{

		}
		else
		{
			CloseSocket();
		}
	}
	else
	{
		//������
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