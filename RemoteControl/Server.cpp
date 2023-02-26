#include "pch.h"
#include "Server.h"
#pragma warning(disable:4407)

CClient::CClient() : m_isbusy(false), m_overlapped(new ACCEPTOVERLAPPED())
{
	m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_buffer.resize(1024);
	memset(&m_laddr, 0, sizeof(m_laddr));
	memset(&m_raddr, 0, sizeof(m_raddr));
}


void CClient::SetOverlapped(PCLIENT& ptr)
{
	m_overlapped->m_client = ptr.get();
}

CClient::operator LPOVERLAPPED()
{
	return &m_overlapped->m_overlapped;
}

template<COperator op>
AcceptOverlapped<op>::AcceptOverlapped()
{
	m_worker = ThreadWorker(this, (FUNCTYPE)&AcceptOverlapped<op>::AcceptWorker);
	m_operator = EAccept;
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
	m_server = NULL;
}

template<COperator op>
int AcceptOverlapped<op>::AcceptWorker()
{
	INT lLength = 0;
	INT rLength = 0;
	if (*(LPDWORD)m_client > 0)
	{
		GetAcceptExSockaddrs(*m_client, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (sockaddr**)m_client->GetLocalAddr(), &lLength, (sockaddr**)m_client->GetRemoteddr(), &rLength);
		if (!m_server->NewAccept())
		{
			return -2;
		}
	}
	return -1;
}