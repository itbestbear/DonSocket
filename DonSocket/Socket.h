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

#ifndef SOCKET_H
#define SOCKET_H

#include "Config.h"

#ifdef _WIN32
	#define FD_SETSIZE      65535 //DON_SOCKET_SELECT
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <WinSock2.h>
	#include <mswsock.h>
	#pragma comment(lib,"ws2_32.lib")
#elif linux
	#include <unistd.h> //uni std
	#include <arpa/inet.h>
	#include <string.h>
	#include <signal.h>
	#include <sys/socket.h>
	#include <sys/epoll.h>
	#include <fcntl.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <errno.h>
	#define EPOLL_ERROR            (-1)
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#elif unix
	#include <unistd.h> //uni std
	#include <arpa/inet.h>
	#include <string.h>
	#include <signal.h>
	#include <sys/socket.h>
	#include <sys/event.h>
	#include <fcntl.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <errno.h>
	#include <stdlib.h>
	#define KQUEUE_ERROR            (-1)
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif


#include <string>

class CSocket
{
public:
	CSocket();
	CSocket(SOCKET hSocket);
	CSocket(SOCKET hSocket,char* addr,size_t port);
	~CSocket();
public:
	static void Initialize();
	static void Shutdown();
public:
	bool Create();
	bool Bind(const char* ip, unsigned short port);
	bool Listen(int n);
	CSocket Accept();
	bool Connect(const char* ip, unsigned short port);
	bool Close();
	int Send(const char* pBuf, size_t nLen);
	int Recv(char* pBuf, size_t nLen);
public:
	bool IsValid() { return m_hSocket != INVALID_SOCKET; }
	bool SetSendBufSize(int& nSize);
	bool SetRecvBufSize(int& nSize);
	bool SetNonBlocking();
	bool SetReuseAddr();
	bool SetNoDelay();
	bool SetUpdateContext(CSocket listen);

public:
	void GetLocalAddr();
	void GetRemoteAddr();
	SOCKET&		 GetSocket(){ return m_hSocket; }
	std::string& GetAddr()	{ return m_strAddr; }
	size_t		 GetPort()	{ return m_nPort;	}
private:
	SOCKET		m_hSocket;
	std::string m_strAddr;
	size_t		m_nPort;
};

#endif // SOCKET_H
