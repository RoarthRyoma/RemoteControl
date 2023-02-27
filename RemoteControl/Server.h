#pragma once
#include "Thread.h"
#include"Queue.h"
#include<map>
#include<MSWSock.h>
#include <ws2tcpip.h>

enum COperator
{
	ENone,
	EAccept,
	ERecv,
	ESend,
	EError
};

class CServer;
class CClient;
typedef std::shared_ptr<CClient> PCLIENT;

class COverlapped
{
public:
	OVERLAPPED m_overlapped;
	DWORD m_operator;	//操作 COperator
	std::vector<char>m_buffer;	//缓冲区
	ThreadWorker m_worker;		//处理函数
	CServer* m_server;		//服务器对象
	CClient* m_client;		//对应的客户端
	WSABUF m_wsabuffer;
};

template<COperator>class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;
template<COperator>class RecvOverlapped;
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;
template<COperator>class SendOverlapped;
typedef SendOverlapped<ESend> SENDOVERLAPPED;

class CClient : public ThreadFuncBase
{
public:
	CClient();
	~CClient() { }

	void SetOverlapped(PCLIENT& ptr);

	operator SOCKET()
	{
		return m_sock;
	}

	operator PVOID()
	{
		return &m_buffer[0];
	}

	operator LPOVERLAPPED();

	operator LPDWORD()
	{
		return &m_received;
	}
	LPWSABUF RecvWSABuffer();

	LPWSABUF SendWSABuffer();

	DWORD& flags()
	{
		return m_flags;
	}

	sockaddr_in* GetLocalAddr()
	{
		return &m_laddr;
	}

	sockaddr_in* GetRemoteddr()
	{
		return &m_raddr;
	}

	size_t GetBufferSize()
	{
		return m_buffer.size();
	}

	int Recv()
	{
		int ret = recv(m_sock, m_buffer.data() + m_used, m_buffer.size() - m_used, 0);
		if (ret <= 0)
		{
			return -1;
		}
		m_used += (size_t)ret;
		return 0;
	}
private:
	SOCKET m_sock;
	DWORD m_received;
	DWORD m_flags;
	std::shared_ptr<ACCEPTOVERLAPPED> m_accept;
	std::shared_ptr<RECVOVERLAPPED> m_recv;
	std::shared_ptr<SENDOVERLAPPED> m_send;
	std::vector<char>m_buffer;
	size_t m_used;		//已经使用的缓冲区大小
	sockaddr_in m_laddr;
	sockaddr_in m_raddr;
	bool m_isbusy;
};

template<COperator>
class AcceptOverlapped :public COverlapped, ThreadFuncBase
{
public:
	AcceptOverlapped();
	int AcceptWorker();
};
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;

template<COperator>
class RecvOverlapped :public COverlapped, ThreadFuncBase
{
public:
	RecvOverlapped();
	int RecvWorker()
	{
		int ret = m_client->Recv();
		return ret;
	}

};
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;

template<COperator>
class SendOverlapped :public COverlapped, ThreadFuncBase
{
public:
	SendOverlapped();
	int SendWorker()
	{
		return -1;
	}
};
typedef SendOverlapped<ESend> SENDOVERLAPPED;

template<COperator>
class ErrorOverlapped :public COverlapped, ThreadFuncBase
{
public:
	ErrorOverlapped() :m_operator(EError), m_worker(this, &ErrorOverlapped::ErrorWorker)
	{
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_buffer.resize(1024);
	}
	int ErrorWorker()
	{
		return -1;
	}
};
typedef ErrorOverlapped<EError> ERROROVERLAPPED;


class CServer : public ThreadFuncBase
{
public:
	CServer(const std::string& ip = "0.0.0.0", short port = 9527):m_pool(10)
	{
		m_hIOCP = INVALID_HANDLE_VALUE;
		m_sock = INVALID_SOCKET;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr.s_addr);
		//InetPton(AF_INET, (PCWSTR)ip.c_str(), &m_addr.sin_addr.s_addr);
		//m_addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());//C4996
	}
	~CServer()
	{

	}
	bool StartService();
	bool NewAccept();
private:
	void CreateSocket()
	{
		m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		int opt = 1;
		setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	}
	int threadIocp();

	CThreadPool m_pool;
	HANDLE m_hIOCP;
	SOCKET m_sock;
	sockaddr_in m_addr;
	std::map<SOCKET, std::shared_ptr<CClient>>m_client;
};

