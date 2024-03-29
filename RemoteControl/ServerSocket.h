#pragma once
#include "pch.h"
#include "framework.h"

#define BUFFER_SIZE 4096

class CPacket
{
public:
	CPacket():sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}

	//包数据解析
	CPacket(const BYTE* pData, size_t& nSize)
	{
		size_t i = 0;
		//解析包头
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF)
			{
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		//不小于包头加长度加控制命令加校验和的总字节数
		if (i + 4 + 2 + 2 >= nSize)//包数据可能不全，或包未能全部接收到
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize)//包未完全接收到，返回解析失败
		{
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[i]) & 0xFF;
		}
		if (sum == sSum)
		{
			nSize = i;//在缓存buffer中,每次都移掉之前的废数据+当前包的长度,所以为i
			return;
		}
		nSize = 0;
	}
	~CPacket(){}
	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
public:
	WORD sHead;			//包头 固定FEFF
	DWORD nLength;		//包长度（从控制命令开始，到和校验结束）
	WORD sCmd;			//控制命令
	std::string strData;//包数据
	WORD sSum;			//和校验

};

class CServerSocket
{
public:
	static CServerSocket* getInstance()
	{
		if (m_instance == NULL)//静态函数没有this指针，所以无法直接访问成员变量
		{
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket()
	{
		//SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);//以TCP流式传输
		if (m_sock == -1)
		{
			return false;
		}
		//校验
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		//绑定
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
		if (m_client == -1) return -1;
		//char buffer[1024]{};
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);//准备好缓冲区
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
			{
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool Send(const char* pData, int nSize)
	{
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
private:
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;

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
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误!"), MB_OK | MB_ICONERROR);
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
		if (WSAStartup(MAKEWORD(1, 1), &data))//返回值处理
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
