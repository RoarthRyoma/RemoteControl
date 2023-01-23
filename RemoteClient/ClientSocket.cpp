#include "pch.h"
#include "ClientSocket.h"

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& listPacket, bool isAutoClosed)
{
	if (m_sock == INVALID_SOCKET)
	{
		//if (!InitSocket()) return false;
		_beginthread(&CClientSocket::threadEntry, 0, this);
	}
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, listPacket));
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	TRACE("cmd:%d event:%08X thread_id:%d\r\n", pack.sCmd, pack.hEvent, GetCurrentThreadId());
	m_listSend.push_back(pack);
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
		m_mapAck.erase(it);
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
			CPacket& head = m_listSend.front();
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
					if (length > 0 || index > 0)
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
							}
						}
					}
					else if (length <= 0 && index <= 0)
					{
						CloseSocket();
						SetEvent(head.hEvent);//�ȵ��������ر��������֪ͨ�¼����
						m_mapAutoClosed.erase(bIt);
						break;
					}
				} while (!bIt->second);
			}
			m_listSend.pop_front();
			if(!InitSocket()) InitSocket();
		}
	}
	CloseSocket();
}

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* that = (CClientSocket*)arg;
	that->threadFunc();
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