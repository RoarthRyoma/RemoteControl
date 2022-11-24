#include "pch.h"
#include "ClientSocket.h"

//CClientSocket server;
CClientSocket* CClientSocket::m_instance = NULL;

CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();