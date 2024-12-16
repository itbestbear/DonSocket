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

#ifndef DON_TCP_SERVER_H
#define DON_TCP_SERVER_H

#include "Config.h"

#ifdef DON_SOCKET_IOCP
#include "Iocp.h"
#elif DON_SOCKET_EPOLL
#include "Epoll.h"
#elif DON_SOCKET_SELECT
#include "Select.h"
#elif DON_SOCKET_KQUEUE
#include "Kqueue.h"
#endif

#include "../Util/Log.h"
#include "MemPool.h"

#include <string>
#include <thread>
#include <mutex>
#include <atomic>

class IDonTcpServerCallBack
{
public:
	virtual ~IDonTcpServerCallBack() = 0;

	virtual int OnAccept(size_t index, int serial, const char* addr, int port) = 0;
	virtual int OnClose(size_t index, int serial, const char* addr, int port) = 0;
	virtual int OnReceive(size_t index, int serial, const void* pdata, size_t len) = 0;
};

inline IDonTcpServerCallBack::~IDonTcpServerCallBack() {}


class CDonTcpServer  
{
private:
	struct SOCK_DATA
	{
		CSocket		socker;
		CBuffer*	pRecvBuf;
		CBuffer*	pSendBuf;
		int			nWorkerId;
	};
	
	struct CONN_DATA
	{
		std::mutex	cdMutex;
		long		nSerial;
		SOCK_DATA*	pSock;
	};
		
	void ListenProc(void * lpParameter);
	void WorkerProc(void * lpParameter, size_t nThreadId);

public:
	explicit CDonTcpServer(IDonTcpServerCallBack* callback,int nMaxClients = 10000,int nMaxThreads = 16,
		int nSendBufSize = 10240, int nRecvBufSize = 10240,
		int nSockSendBufSize = 0, int nSockRecvBufSize = 0);
	virtual ~CDonTcpServer();

	void SetCallBack(IDonTcpServerCallBack * callback);
	IDonTcpServerCallBack * GetCallBack() const;

	bool Initialize(const char * addr, int port);
	bool Shutdown();
	bool ClientConnected(size_t index, int serial);
	bool ClientSend(size_t index, int serial, const void * pdata, size_t len);
	bool ClientSend2(size_t index, int serial,  const void * pdata1, size_t len1, const void * pdata2, size_t len2);
	bool Broadcast(const void * pdata, size_t len);
	bool Broadcast2(const void * pdata1, size_t len1,const void * pdata2, size_t len2);
	bool ClientClose(size_t index, int serial);
	void CloseAll();
	std::string Dump(size_t index);
public:
	size_t	GetConnectNum() const		{ return m_nCount; } 
	size_t	GetMaxConnectNum() const	{ return m_nConnectNum; } 
	size_t	GetWorkerNum() const		{ return m_nWorkerNum; }
	bool	GetConnected(size_t index)	{ return (m_pConnects[index].pSock != NULL); }

private:	
	CDonTcpServer(const CDonTcpServer &);
	CDonTcpServer & operator=(const CDonTcpServer &);
	
	CDonTcpServer::SOCK_DATA * LockSock(size_t index);
	void UnlockSock(size_t index);
	CDonTcpServer::SOCK_DATA* GetSock(size_t index);

	bool Accept(CSocket sock);
	long GetSerial();
	int NewConnect();
	void DeleteConnect(size_t index);
	bool Close(size_t index);
	
private:
	bool ProcessRead(size_t index, size_t size);
	bool ProcessWrite(size_t index, size_t size);

private:
	IDonTcpServerCallBack*	m_pCallBack;
	bool					m_bQuit;

private:
	CSocket				m_hListenSock;
	std::thread			m_hListenThread;
	std::thread*		m_hWorkerThreads;

#ifdef DON_SOCKET_IOCP
	CIocp				m_hListenIocp;
	CIocp*				m_hWorkerIocps;
#elif DON_SOCKET_EPOLL
	CEpoll				m_hListenEpoll;
	CEpoll*          	m_hWorkerEpolls;
#elif DON_SOCKET_SELECT
	CSelect				m_hListenSelect;
	CSelect*			m_hWorkerSelects;
#elif DON_SOCKET_KQUEUE
	CKqueue				m_hListenKqueue;
	CKqueue*			m_hWorkerKqueues;
#endif

	size_t				m_nConnectNum;	//maximum connection num
	std::atomic<size_t>	m_nCount;		//current connection num
	size_t				m_nWorkerNum;	//threads num
	size_t              m_nCapacityNum;	//each iocp/epoll/select handle m_nCapaciyNum fd

	int					m_nSendBufSize;	//local send buf size
	int 				m_nRecvBufSize;	//local recv buf size

	int					m_nSockSendBufSize;	//system send buf size
	int					m_nSockRecvBufSize;	//system recv buf size

	CONN_DATA *			m_pConnects;	//all connections
	
private:
	std::atomic<long>	m_nSerial;

public:
	size_t GetSockPoolSize() { return m_pool_sock.Count(); }
	size_t GetSendPoolSize() { return m_pool_sendbuf.Count(); }
	size_t GetRecvPoolSize() { return m_pool_recvbuf.Count(); }
private:
	CMemPool<SOCK_DATA> m_pool_sock;
	CMemPool<CBuffer>	m_pool_sendbuf;
	CMemPool<CBuffer>	m_pool_recvbuf;
};

#endif // DON_TCP_SERVER_H
