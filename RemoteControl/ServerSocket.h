#pragma once
#include "pch.h"
#include "framework.h"


class CServerSocket
{
public:
	static CServerSocket* getInstance()
	{
		if (m_instance == NULL)//��̬����û��thisָ�룬�����޷�ֱ�ӷ��ʳ�Ա����
		{
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket()
	{
		//SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);//��TCP��ʽ����
		if (m_sock == -1)
		{
			return false;
		}
		//У��
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		//��
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		{
			return false;
		}

		if (listen(m_sock, 1) == -1)
		{
			return false;
		}
		return true;
	}

	bool AcceptClient()
	{
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1) return false;
		return true;
	}

	int DealCommand()
	{
		char buffer[1024]{};
		while (true)
		{
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0)
			{
				return -1;
			}
		}
	}

	bool Send(const char* pData, int nSize)
	{
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
private:
	SOCKET m_sock;
	SOCKET m_client;

	CServerSocket& operator=(const CServerSocket& ss){}

	CServerSocket(const CServerSocket& s)
	{
		m_sock = s.m_sock;
		m_client = s.m_client;
	}

	CServerSocket()
	{
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ����������������ã�"), _T("��ʼ������!"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}

	~CServerSocket()
	{
		closesocket(m_sock);
		WSACleanup();
	}

	BOOL InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data))//����ֵ����
		{
			return FALSE;
		}
		return TRUE;
	}

	static void ReleaseInstance()
	{
		if (m_instance != NULL)
		{
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	static CServerSocket* m_instance;

	class CHelper
	{
	public:
		CHelper()
		{
			CServerSocket::getInstance();
		}
		~CHelper()
		{
			CServerSocket::ReleaseInstance();
		}
	};

	static CHelper m_helper;
};
