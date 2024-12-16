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

#ifndef DON_TCP_CLIENT_H
#define DON_TCP_CLIENT_H

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
#include <atomic>
#include <mutex>

class IDonTcpClientCallBack
{
public:
	virtual ~IDonTcpClientCallBack() = 0;

	virtual int OnConnect(size_t index, const char* addr, int port) = 0;
	virtual int OnClose(size_t index, const char* addr, int port) = 0;
	virtual int OnReceive(size_t index, const void* pdata, size_t len) = 0;
};

inline IDonTcpClientCallBack::~IDonTcpClientCallBack() {}

class CDonTcpClient  
{
private:
	enum SOCK_STATE
	{
		SS_INIT,
		SS_CONNECTED,
		SS_FAILED,
		SS_WAITING,
	};
	
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
		int			nState;
		size_t		nCount;
		SOCK_DATA*	pSock;
	};
	
	void ConnectProc(void* lpParameter);
	void WorkerProc(void * lpParameter, size_t nThreadId);

public:
	explicit CDonTcpClient(IDonTcpClientCallBack* callback,int nMaxConnects = 10000,int nMaxThreads=16,
		int nSendBufSize = 10240, int nRecvBufSize = 10240,
		int nSockSendBufSize = 0, int nSockRecvBufSize = 0);
	virtual ~CDonTcpClient();

	void SetCallBack(IDonTcpClientCallBack * callback);
	IDonTcpClientCallBack * GetCallBack() const;

	bool Initialize(const char * addr, int port, bool bStart=true);
	bool Shutdown();
	bool ClientConnected(size_t index);
	bool ClientSend(size_t index, const void * pdata, size_t len);
	bool ClientSend2(size_t index, const void * pdata1,size_t len1, const void * pdata2, size_t len2);
	bool ClientSendAll(const void* pdata, size_t len);
	bool ClientSendAll2(const void* pdata1, size_t len1, const void* pdata2, size_t len2);
	bool ClientClose(size_t index);
	void CloseAll();
	std::string Dump(size_t index);
public:
	size_t	GetConnectNum() const		{ return m_nCount; } 
	size_t	GetMaxConnectNum() const	{ return m_nConnectNum; }
	size_t	GetWorkerNum() const		{ return m_nWorkerNum; }
	bool	GetConnected(size_t index)	{ return (m_pConnects[index].pSock != NULL); }
	
public:
	bool IsStart() const;
	bool Start();
	bool Stop();

private:
	CDonTcpClient(const CDonTcpClient &);
	CDonTcpClient & operator=(const CDonTcpClient &);
	
	CDonTcpClient::SOCK_DATA * LockSock(size_t index);
	void UnlockSock(size_t index);
	CDonTcpClient::SOCK_DATA* GetSock(size_t index);

	CDonTcpClient::SOCK_DATA* NewConnect(size_t index);
	void DeleteConnect(size_t index);

	void CheckConnect(size_t index);
	void SwitchState(size_t index, int state);
	int GetState(size_t index);
	bool Close(size_t index);

private:
	bool ProcessRead(size_t index, size_t size);
	bool ProcessWrite(size_t index, size_t size);

private:
	IDonTcpClientCallBack*	m_pCallBack;
	bool					m_bQuit;

private:
	std::thread			m_hConnectThread;
	std::thread*		m_hWorkerThreads;

#ifdef DON_SOCKET_IOCP
	CIocp*				m_hWorkerIocps;
#elif DON_SOCKET_EPOLL
	CEpoll*				m_hWorkerEpolls;
#elif DON_SOCKET_SELECT
	CSelect*			m_hWorkerSelects;
#elif DON_SOCKET_KQUEUE
	CKqueue*			m_hWorkerKqueues;
#endif

	size_t 				m_nConnectNum;	//maximum connection num
	std::atomic<size_t>	m_nCount;		//current connection num
	size_t				m_nWorkerNum;	//threads num
	size_t				m_nCapacityNum;	//each iocp/epoll/select handle m_nCapaciyNum fd

	int					m_nSendBufSize; //local send buf size
	int					m_nRecvBufSize;	//local recv buf size

	int					m_nSockSendBufSize; //system send buf size
	int					m_nSockRecvBufSize; //system recv buf size

	CONN_DATA *			m_pConnects;	//all connections

private:
	std::string			m_strDestAddr;
	unsigned short		m_nDestPort;
	std::atomic<long>	m_Start;
	
public:
	size_t GetSockPoolSize() { return m_pool_sock.Count(); }
	size_t GetSendPoolSize() { return m_pool_sendbuf.Count(); }
	size_t GetRecvPoolSize() { return m_pool_recvbuf.Count(); }
private:
	CMemPool<SOCK_DATA> m_pool_sock;
	CMemPool<CBuffer>	m_pool_sendbuf;
	CMemPool<CBuffer>	m_pool_recvbuf;
};

#endif // DON_TCP_CLIENT_H
