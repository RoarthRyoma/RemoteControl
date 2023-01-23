#include "pch.h"
#include "ClientSocket.h"

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	while (m_sock != INVALID_SOCKET)
	{
		if (m_listSend.size() > 0)
		{
			TRACE("listSend size: %d\r\n", m_listSend.size());
			CPacket& head = m_listSend.front();
			if (Send(head) == false)
			{
				TRACE("发送失败!\r\n");
				continue;
			}
			auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));
			int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
			if (!(length <= 0 && index <= 0))
			{
				index += length;
				size_t size = (size_t)index;
				CPacket pack((BYTE*)pBuffer, size);
				if (size > 0)
				{//TODO:文件夹信息获取，文件信心获取可能产生问题
					pack.hEvent = head.hEvent;
					pr.first->second.push_back(pack);
					SetEvent(head.hEvent);
				}
			}
			else
			{
				CloseSocket();
			}
			m_listSend.pop_front();
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