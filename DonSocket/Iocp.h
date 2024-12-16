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

#ifndef IOCP_H
#define IOCP_H

#include "Buffer.h"

#ifdef DON_SOCKET_IOCP

enum IOCP_EVENT_TYPE
{
	IOCP_EVENT_ACCEPT,
	IOCP_EVENT_RECV,
	IOCP_EVENT_SEND,
};

struct IOCP_DATA_BASE
{
	OVERLAPPED	overlapped;
	SOCKET		sockfd;
	WSABUF		wsabuff;
	int			iotype;	//IOCP_EVENT_TYPE
};

struct IOCP_EVENT
{
	union
	{
		void*  pVoid;
		SOCKET sockfd;
	}data;
	IOCP_DATA_BASE* pIoData;
	DWORD bytesTrans = 0;
};

class CIocp
{
public:
	~CIocp();

public:
	bool Create();
	void Destory();
	int  Wait(int timeout_ms);

	bool Operatei(CSocket sockfd);
	bool Operatei(CSocket sockfd, void* pVoid);
	bool Operatei(CSocket sockfd, size_t index);

public:
	bool PostAccept(IOCP_DATA_BASE* pIO_DATA);
	bool PostReceive(IOCP_DATA_BASE* pIO_DATA);
	bool PostSend(IOCP_DATA_BASE* pIO_DATA);

public:
	bool LoadAcceptEx(CSocket ListenSocket);
	
private:
	LPFN_ACCEPTEX	m_AcceptEx = NULL;
	HANDLE			m_hCompletionPort = NULL;
	CSocket			m_sockServer;

public:
	IOCP_EVENT* GetOutEvents() { return &m_ioEvent; }
private:
	IOCP_EVENT	m_ioEvent;
};

#endif //DON_SOCKET_IOCP

#endif // IOCP_H
