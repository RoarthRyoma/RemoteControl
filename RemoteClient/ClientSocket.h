#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <list>
#include <map>

#pragma pack(push)
#pragma pack(1)

#define BUFFER_SIZE 4096000
class CPacket
{
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0), hEvent(INVALID_HANDLE_VALUE)
	{
	}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		hEvent = pack.hEvent;
	}

	//封包
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize, HANDLE event)
	{
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
		{
			strData.clear();
		}

		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
		this->hEvent = event;
	}

	//包数据解析
	CPacket(const BYTE* pData, size_t& nSize) : CPacket()
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
		if (i + 4 + 2 + 2 > nSize)//包数据可能不全，或包未能全部接收到
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
			TRACE("客户端接收包: %s\r\n", strData.c_str());
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			nSize = i;//在缓存buffer中,每次都移掉之前的废数据+当前包的长度,所以为i
			return;
		}
		nSize = 0;
	}

	~CPacket()
	{
	}

	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
			hEvent = pack.hEvent;
		}
		return *this;
	}

	//包数据的大小
	int Size()
	{
		return nLength + 6;
	}

	//包数据内容
	const char* Data(std::string& strOut) const
	{
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;
		pData += 2;

		*(DWORD*)(pData) = nLength;
		pData += 4;

		*(WORD*)pData = sCmd;
		pData += 2;

		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();

		*(WORD*)pData = sSum;

		return strOut.c_str();
	}

public:
	WORD sHead;			//包头 固定FEFF
	DWORD nLength;		//包长度（从控制命令开始，到和校验结束）
	WORD sCmd;			//控制命令
	std::string strData;//包数据
	WORD sSum;			//和校验
	//std::string strOut; //整个包的数据
	HANDLE hEvent;
};

#pragma pack(pop)

typedef struct MouseEvent
{
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击-0、双击-1、按下-2、弹起-3、移动不显示按键
	WORD nButton;//左键、右键、中键
	POINT ptXY;//坐标
}MOUSEEV, * PMOUSEEV;

typedef struct _FILE_INFO
{
	_FILE_INFO()
	{
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}

	BOOL IsInvalid;//是否有效, 0-否 1-是
	BOOL IsDirectory;//是否为目录, 0-否 1-是
	BOOL HasNext;   //是否还有后续
	char szFileName[256];//文件名

} FILEINFO, * PFILEINFO;

std::string GetErrorInfo(int wsaErrCode);

class CClientSocket
{
public:
	static CClientSocket* getInstance()
	{
		if (m_instance == NULL)//静态函数没有this指针，所以无法直接访问成员变量
		{
			m_instance = new CClientSocket();
			TRACE("CCLientSocket size is %d\r\n", sizeof(*m_instance));
		}
		return m_instance;
	}

	bool InitSocket()
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
		return true;
	}

	int DealCommand()
	{
		if (m_sock == -1) return -1;
		//char buffer[1024]{};
		//char* buffer = new char[BUFFER_SIZE];
		char* buffer = m_buffer.data();//多线程发送命令是可能会出现冲突
		//memset(buffer, 0, BUFFER_SIZE);//准备好缓冲区
		static size_t index = 0;
		while (true)
		{
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			if (((int)len <= 0) && ((int)index == 0))
			{
				return -1;
			}
			TRACE("recv len = %d(0x%08X) index = %d(0x%08X)\r\n", len, len, index, index);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, index - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool Send(const char* pData, int nSize)
	{
		if (m_sock == -1) return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(const CPacket& pack)
	{
		TRACE("m_sock = %d\r\n", m_sock);
		if (m_sock == -1) return false;
		std::string strOut;
		pack.Data(strOut);
		return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
	}
	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4))
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& GetPacket()
	{
		return m_packet;
	}
	void CloseSocket()
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	void UpdateAddress(int nIP, int nPort)
	{
		m_nIP = nIP;
		m_nPort = nPort;
	}
private:
	std::list<CPacket> m_listSend;
	std::map<HANDLE, std::list<CPacket>> m_mapAck;
	int m_nIP;
	int m_nPort;//端口
	SOCKET m_sock;
	CPacket m_packet;
	std::vector<char> m_buffer;

	CClientSocket& operator=(const CClientSocket& ss)
	{
	}

	CClientSocket(const CClientSocket& s) 
	{
		m_sock = s.m_sock;
		m_nIP = s.m_nIP;
		m_nPort = s.m_nPort;
	}

	CClientSocket() : m_nIP(INADDR_ANY), m_nPort(0)
	{
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化Socket环境，请检查网络设置！"), _T("初始化错误!"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		//m_sock = socket(PF_INET, SOCK_STREAM, 0);
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
	}

	~CClientSocket()
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
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

	void threadFunc();
	static void threadEntry(void* arg);

	static void ReleaseInstance()
	{
		TRACE("CClientSocket has been called!\r\n");
		if (m_instance != NULL)
		{
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
			TRACE("CClientSocket has released!\r\n");
		}
	}

	static CClientSocket* m_instance;

	class CHelper
	{
	public:
		CHelper()
		{
			CClientSocket::getInstance();
		}
		~CHelper()
		{
			CClientSocket::ReleaseInstance();
		}
	};

	static CHelper m_helper;
};
