#pragma once
#include "Thread.h"
#include"Queue.h"
#include<map>
#include<MSWSock.h>
#include <ws2tcpip.h>
#include "EdoyunTool.h"

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
	DWORD m_operator;	//���� COperator
	std::vector<char>m_buffer;	//������
	ThreadWorker m_worker;		//������
	CServer* m_server;		//����������
	CClient* m_client;		//��Ӧ�Ŀͻ���
	WSABUF m_wsabuffer;
	virtual ~COverlapped()
	{
		m_buffer.clear();
	}
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
	~CClient()
	{
		m_buffer.clear();
		closesocket(m_sock);
		m_recv.reset();
		m_send.reset();
		m_accept.reset();
		m_vecSend.Clear();
	}

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

	LPWSAOVERLAPPED RecvOverlapped();

	LPWSABUF SendWSABuffer();

	LPWSAOVERLAPPED SendOverlapped();

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

	int Recv();

	int Send(void* buffer, size_t nSize);

	int SendData(std::vector<char>& data);
private:
	SOCKET m_sock;
	DWORD m_received;
	DWORD m_flags;
	std::shared_ptr<ACCEPTOVERLAPPED> m_accept;
	std::shared_ptr<RECVOVERLAPPED> m_recv;
	std::shared_ptr<SENDOVERLAPPED> m_send;
	std::vector<char>m_buffer;
	size_t m_used;		//�Ѿ�ʹ�õĻ�������С
	sockaddr_in m_laddr;
	sockaddr_in m_raddr;
	bool m_isbusy;
	CSendQueue<std::vector<char>> m_vecSend;	//�������ݶ���
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
		m_addr.sin_family = AF_INET;//���ﲻ���ã�����ĺ���������ȷ����sin_family,��֪��ʲôԭ��
		m_addr.sin_port = htons(port);
		INT res = inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr.S_un.S_addr);
		//InetPton(AF_INET, (PCWSTR)ip.c_str(), &m_addr.sin_addr.s_addr);
		//m_addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());//C4996
		if (res != 1)
		{
			//����ֵΪ1��˵���ַ���ip���ɹ�ת������IP��ַ�����洢����addrָ��Ľṹ���С�
			//����ֵΪ0��˵���ַ���ip���ǺϷ���IP��ַ��ʽ���޷�ת����
			//����ֵΪ - 1��˵������ִ��ʧ�ܣ�ԭ������ǵ�ַ�岻֧�֡���ַת��ʧ�ܵȡ�
			TRACE("inet_pton->res: %d\r\n", res);
		}
	}
	~CServer();
	bool StartService();
	bool NewAccept();
	void BindNewSocket(SOCKET s);
private:
	void CreateSocket()
	{
		WSADATA WSAData;
		WSAStartup(MAKEWORD(2, 2), &WSAData);
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

