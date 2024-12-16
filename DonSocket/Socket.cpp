/* Copyright (C) XiongZG.
 * This file is part of DonSocket
 *
 * DonSocket is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * DonSocket is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DonSocket.If not, see <http://www.gnu.org/licenses/>.
 */

#include "Socket.h"

#include "../Util/Log.h"

CSocket::CSocket()
{
	m_hSocket = INVALID_SOCKET;
	m_nPort = -1;
}

CSocket::CSocket(SOCKET hSocket)
{
	m_hSocket = hSocket;
	m_nPort = -1;
}

CSocket::CSocket(SOCKET hSocket, char* addr, size_t port)
{
	m_hSocket = hSocket;
	m_strAddr = addr;
	m_nPort = port;
}

CSocket::~CSocket()
{
	//CSocket按值传递，析构不可以关闭socket，socket需要手动明确调Close关闭
}

void CSocket::Initialize()
{
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#else
	//if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	//	return (1);
	//忽略异常信号，默认情况会导致进程终止
	signal(SIGPIPE, SIG_IGN);
#endif
}

void CSocket::Shutdown()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

bool CSocket::Create()
{
	if (INVALID_SOCKET != m_hSocket)
	{
		Close();
	}
#ifdef _unix
	m_hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
	if (INVALID_SOCKET == m_hSocket)
	{
		return false;
	}
		
	SetReuseAddr();
	
	return true;
}

//绑定IP和端口号
bool CSocket::Bind(const char* ip, unsigned short port)
{
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);//host to net unsigned short

#ifdef _WIN32
	if (ip) {
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
	}
	else {
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	}
#else
	if (ip) {
		_sin.sin_addr.s_addr = inet_addr(ip);
	}
	else {
		_sin.sin_addr.s_addr = INADDR_ANY;
	}
#endif
	int ret = bind(m_hSocket, (sockaddr*)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == ret)
	{
		//log

		Close();

		return false;
	}

	return true;
}

bool CSocket::Listen(int n)
{
	int ret = listen(m_hSocket, n);
	if (SOCKET_ERROR == ret)
	{
		//log

		Close();

		return false;
	}
	
	return true;
}

CSocket CSocket::Accept()
{
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
	cSock = accept(m_hSocket, (sockaddr*)&clientAddr, &nAddrLen);
#else
	cSock = accept(m_hSocket, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
	if (INVALID_SOCKET == cSock)
	{
		//log
		return CSocket();
	}

	char* addr = ::inet_ntoa(clientAddr.sin_addr);
	size_t port = ntohs(clientAddr.sin_port);

	CSocket sock(cSock, addr, port);
	sock.SetReuseAddr();

	return sock;
}

bool CSocket::Connect(const char* ip, unsigned short port)
{
	if (INVALID_SOCKET == m_hSocket)
		return false;
	
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif
	
	int ret = connect(m_hSocket, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		//log
		return false;
	}

	//GetRemoteAddr();
	GetLocalAddr();

	return true;
}

bool CSocket::Close()
{
	int ret = 0;
	if (m_hSocket)
	{
#ifdef _WIN32
		ret = closesocket(m_hSocket);
#else
		ret = close(m_hSocket);
#endif
	}

	m_hSocket = INVALID_SOCKET;

	if (ret == SOCKET_ERROR)
		return false;

	return true;
}

int CSocket::Send(const char* pBuf, size_t nLen)
{
	int nRet = send(m_hSocket, pBuf, nLen, 0);
	if (nRet == SOCKET_ERROR)
	{
		//printf("send failed: %d\n", WSAGetLastError());
	}

	return nRet;//Bytes sent
}

int CSocket::Recv(char* pBuf, size_t nLen)
{
	int nRet = recv(m_hSocket, pBuf, nLen, 0);

	if (nRet > 0)
	{
		//printf("Bytes received: %d\n", iResult);
	}
	else if (nRet == 0)
	{
#ifdef _WIN32
		int nErr = WSAGetLastError();
		log_trace_error("CSocket::Recv,len:%d err:%d,Close", nRet, nErr);
#else
		log_trace_error("CSocket::Recv,len:%d err:%d,Close", nRet, errno);
#endif
		//printf("Connection closed\n");
	}
	else
	{
#ifdef _WIN32
		int nErr = WSAGetLastError();
		log_trace_error("recv failed: len:%d err:%d", nRet,nErr);
#else
		log_trace_error("recv failed: len:%d err:%d", nRet, errno);
#endif
	}
	return nRet;
}

void CSocket::GetLocalAddr()
{
	struct sockaddr_in laddr;

#ifdef _WIN32
	int laddr_len = sizeof(sockaddr_in);
#else
	socklen_t laddr_len = sizeof(sockaddr_in);
#endif

	getsockname(m_hSocket, (struct sockaddr*)&laddr, &laddr_len);

	m_strAddr = inet_ntoa(laddr.sin_addr);
	m_nPort	  = ntohs(laddr.sin_port);
}

void CSocket::GetRemoteAddr()
{
	struct sockaddr_in raddr;

#ifdef _WIN32
	int raddr_len = sizeof(sockaddr_in);
#else
	socklen_t raddr_len = sizeof(sockaddr_in);
#endif

	getpeername(m_hSocket, (struct sockaddr*)&raddr, &raddr_len);
	
	m_strAddr = inet_ntoa(raddr.sin_addr);
	m_nPort = ntohs(raddr.sin_port);
}

bool CSocket::SetSendBufSize(int& nSize)
{
	if (nSize <= 0)
		return true;

	return (::setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nSize, sizeof(int)) != SOCKET_ERROR);
}

bool CSocket::SetRecvBufSize(int& nSize)
{
	if (nSize <= 0)
		return true;

	return (::setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nSize, sizeof(int)) != SOCKET_ERROR);
}

bool CSocket::SetNonBlocking()
{
#ifdef _WIN32
	{
		unsigned long nonblocking = 1;
		if (ioctlsocket(m_hSocket, FIONBIO, &nonblocking) == SOCKET_ERROR)
		{
			return false;
		}
	}
#else
	{
		int flags;
		if ((flags = fcntl(m_hSocket, F_GETFL, NULL)) < 0) 
		{
			return false;
		}
		if (!(flags & O_NONBLOCK)) 
		{
			if (fcntl(m_hSocket, F_SETFL, flags | O_NONBLOCK) == -1) 
			{
				return false;
			}
		}
	}
#endif
	return true;
}

bool CSocket::SetReuseAddr()
{
	int flag = 1;
	if (SOCKET_ERROR == setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag))) 
	{
		return false;
	}
	return true;
}

bool CSocket::SetNoDelay()
{
	int flag = 1;
	if (SOCKET_ERROR == setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag))) 
	{
		return false;
	}
	return true;
}

bool CSocket::SetUpdateContext(CSocket listen)
{
#ifdef _WIN32
	if (SOCKET_ERROR == setsockopt(m_hSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(listen.GetSocket()), sizeof(SOCKET)))
	{
		return false;
	}
#endif
	return true;
}
