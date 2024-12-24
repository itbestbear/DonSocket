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

#include "DonTcpServer.h"

void CDonTcpServer::ListenProc(void* lpParameter)
{
	CDonTcpServer* pthis = (CDonTcpServer*)lpParameter;
#ifdef DON_SOCKET_IOCP
	//PostAccept
	const int len = 1024;
	char buf[len] = {};
	IOCP_DATA_BASE ioData = {};
	ioData.wsabuff.buf = buf;
	ioData.wsabuff.len = len;
	m_hListenIocp.PostAccept(&ioData);

	while (!pthis->m_bQuit)
	{
		int nIocpEvCount = pthis->m_hListenIocp.Wait(1);
		if (nIocpEvCount < 0)
		{
			log_trace_error("[listenproc] m_hListenIocp.Wait failed:%d", GetLastError());
			return;
		}
		else if (nIocpEvCount == 0)
		{
			continue;
		}

		IOCP_EVENT* pEvent = m_hListenIocp.GetOutEvents();

		//接受连接 completion
		if (IOCP_EVENT_ACCEPT == pEvent->pIoData->iotype)
		{
			CSocket sock(pEvent->pIoData->sockfd);

			if (sock.IsValid())
			{
				sock.SetUpdateContext(m_hListenSock);
				sock.GetRemoteAddr();

				pthis->Accept(sock);
			}
			//继续 向IOCP投递接受连接任务
			m_hListenIocp.PostAccept(&ioData);
		}
	}

#elif DON_SOCKET_EPOLL
	while (!pthis->m_bQuit)
	{
		int nEpollEvCount = pthis->m_hListenEpoll.Wait(1);
		if (nEpollEvCount < 0)
		{
			log_trace_error("[listenproc] m_hListenEpoll.Wait failed:%d", errno);
			return;
		}
		else if (nEpollEvCount == 0)
		{
			continue;
		}

		epoll_event* events = pthis->m_hListenEpoll.GetOutEvents();

		if (events->data.fd == m_hListenSock.GetSocket())
		{
			if (events->events & EPOLLIN)
			{
				CSocket sock = m_hListenSock.Accept();

				if (sock.IsValid())
				{
					//m_hListenSock.Accept()中已经获取对端地址
					//sock.SetUpdateContext(m_hListenSock);
					//sock.GetRemoteAddr();

					pthis->Accept(sock);
				}
			}
		}
	}
#elif DON_SOCKET_SELECT
	while (!pthis->m_bQuit)
	{
		//select调用会清空fdset，在wait前需重新注册
		m_hListenSelect.Operates(m_hListenSock, 0, SELECT_OP_MOD, SELECT_READ);

		int nSelectEvCount = pthis->m_hListenSelect.Wait(1);
		if (nSelectEvCount < 0)
		{
#ifdef _WIN32
			log_trace_error("[listenproc] m_hListenSelect.Wait failed:%d", GetLastError());
#else
			log_trace_error("[listenproc] m_hListenSelect.Wait failed:%d", errno);
#endif
			return;
		}
		else if (nSelectEvCount == 0)
		{
			continue;
		}

		SELECT_EVENT* events = pthis->m_hListenSelect.GetOutEvents();
		SELECT_EVENT& event = events[0];

		if (event.sockfd == m_hListenSock.GetSocket())
		{
			if (event.eventType & SELECT_READ)
			{
				CSocket sock = m_hListenSock.Accept();

				if (sock.IsValid())
				{
					//m_hListenSock.Accept()中已经获取对端地址
					//sock.SetUpdateContext(m_hListenSock);
					//sock.GetRemoteAddr();

					pthis->Accept(sock);
				}
			}
		}
	}
#elif DON_SOCKET_KQUEUE
	while (!pthis->m_bQuit)
	{
		int nKqueueEvCount = pthis->m_hListenKqueue.Wait(1);
		if (nKqueueEvCount < 0)
		{
			log_trace_error("[listenproc] m_hListenKqueue.Wait failed:%d", errno);
			return;
		}
		else if (nKqueueEvCount == 0)
		{
			continue;
		}

		struct kevent* events = pthis->m_hListenKqueue.GetOutEvents();

		if ((int)(events->udata) == m_hListenSock.GetSocket())
		{
			if (events->filter == EVFILT_READ)
			{
				CSocket sock = m_hListenSock.Accept();

				if (sock.IsValid())
				{
					//m_hListenSock.Accept()中已经获取对端地址
					//sock.SetUpdateContext(m_hListenSock);
					//sock.GetRemoteAddr();

					pthis->Accept(sock);
				}
			}
		}
	}
#endif
}

void CDonTcpServer::WorkerProc(void* lpParameter, size_t nThreadId)
{
	CDonTcpServer* pthis = (CDonTcpServer*)lpParameter;

#ifdef DON_SOCKET_IOCP
	while (!m_bQuit)
	{
		CIocp& iocp = m_hWorkerIocps[nThreadId];

		//************************************************************************************
		//订阅事件
		for (size_t i = 0; i < m_nCapacityNum; i++)
		{
			size_t index = i + m_nCapacityNum * nThreadId;

			SOCK_DATA* pSock = LockSock(index);
			if (pSock == NULL)
			{
				continue;
			}

			//投递写
			if (pSock->pSendBuf->NeedWrite())
			{
				auto pSendIoData = pSock->pSendBuf->BuildSendIoData(pSock->socker);
				if (pSendIoData)
				{
					if (!iocp.PostSend(pSendIoData))
					{
						UnlockSock(index);
						Close(index);
						continue;
					}
				}
			}

			//投递读
			auto pRecvIoData = pSock->pRecvBuf->BuildRecvIoData(pSock->socker);
			if (pRecvIoData)
			{
				if (!iocp.PostReceive(pRecvIoData))
				{
					UnlockSock(index);
					Close(index);
					continue;
				}
			}

			UnlockSock(index);
		}

		//************************************************************************************
		//处理事件
		int nIocpEvCount = iocp.Wait(1);
		if (nIocpEvCount < 0)
		{
			log_trace_error("[workproc] iocp.Wait failed:%d", GetLastError());
			return;
		}
		else if (nIocpEvCount == 0)
		{
			continue;
		}

		IOCP_EVENT* pEvent = iocp.GetOutEvents();

		size_t nIndex = (size_t)pEvent->data.pVoid;
		if (pEvent->pIoData == NULL || pEvent->bytesTrans < 0) //客户端断开处理
		{
			Close(nIndex);
		}
		else
		{
			switch (pEvent->pIoData->iotype)
			{
			case IOCP_EVENT_SEND: //发送数据 完成 Completion
				pthis->ProcessWrite(nIndex, pEvent->bytesTrans);
				break;
			case IOCP_EVENT_RECV: //接收数据 完成 Completion
				pthis->ProcessRead(nIndex, pEvent->bytesTrans);
				break;
			default:
				break;
			}
		}
	}
#elif DON_SOCKET_EPOLL
	while (!pthis->m_bQuit)
	{
		CEpoll& epoll = m_hWorkerEpolls[nThreadId];

		//************************************************************************************
		//订阅事件
		for (size_t i = 0; i < m_nCapacityNum; i++)
		{
			size_t index = i + m_nCapacityNum * nThreadId;

			SOCK_DATA* pSock = LockSock(index);
			if (pSock == NULL)
			{
				continue;
			}
			//需要写数据的客户端,才加入EPOLLOUT检测是否可写
			if (pSock->pSendBuf->NeedWrite())
			{
				epoll.Operatee(pSock->socker, index, EPOLL_CTL_MOD, EPOLLIN | EPOLLOUT);
			}
			else
			{
				epoll.Operatee(pSock->socker, index, EPOLL_CTL_MOD, EPOLLIN);
			}
			UnlockSock(index);
		}

		//************************************************************************************
		//处理事件
		int nEpollEvCount = epoll.Wait(1);
		if (nEpollEvCount < 0)
		{
			log_trace_error("[workproc] epoll.Wait failed:%d", errno);
			return;
		}
		else if (nEpollEvCount == 0)
		{
			continue;
		}

		epoll_event* epevents = epoll.GetOutEvents();
		for (int i = 0; i < nEpollEvCount; i++)
		{
			epoll_event& ev = epevents[i];

			int index = ev.data.fd;

			if (ev.events & EPOLLIN)
			{
				ProcessRead(index, 1);
			}

			if (ev.events & EPOLLOUT)
			{
				ProcessWrite(index, 1);
			}
		}
	}
#elif DON_SOCKET_SELECT
	while (!pthis->m_bQuit)
	{
		CSelect& select = m_hWorkerSelects[nThreadId];

		//************************************************************************************
		//订阅事件
		select.SetBaseIndex(m_nCapacityNum * nThreadId);

		for (size_t i = 0; i < m_nCapacityNum; i++)
		{
			size_t index = i + m_nCapacityNum * nThreadId;

			SOCK_DATA* pSock = LockSock(index);
			if (pSock == NULL)
			{
				continue;
			}
			//需要写数据的客户端,才加入EPOLLOUT检测是否可写
			if (pSock->pSendBuf->NeedWrite())
			{
				select.Operates(pSock->socker, index, SELECT_OP_MOD, SELECT_READ | SELECT_WRITE);
			}
			else
			{
				select.Operates(pSock->socker, index, SELECT_OP_MOD, SELECT_READ);
			}
			UnlockSock(index);
		}

		//************************************************************************************
		//处理事件
		int nSelectEvCount = select.Wait(1);
		if (nSelectEvCount < 0)
		{
#ifdef _WIN32
			log_trace_error("[workproc] select.Wait failed:%d", GetLastError());
#else
			log_trace_error("[workproc] select.Wait failed:%d", errno);
#endif
			return;
		}
		else if (nSelectEvCount == 0)
		{
			continue;
		}

		SELECT_EVENT* slevents = select.GetOutEvents();
		for (int i = 0; i < nSelectEvCount; i++)
		{
			SELECT_EVENT& ev = slevents[i];

			int index = ev.data.index;

			if (ev.eventType & SELECT_READ)
			{
				ProcessRead(index, 1);
			}

			if (ev.eventType & SELECT_WRITE)
			{
				ProcessWrite(index, 1);
			}
		}
	}
#elif DON_SOCKET_KQUEUE
	while (!pthis->m_bQuit)
	{
		CKqueue& kqueue = m_hWorkerKqueues[nThreadId];

		//************************************************************************************
		//订阅事件
		for (size_t i = 0; i < m_nCapacityNum; i++)
		{
			size_t index = i + m_nCapacityNum * nThreadId;

			SOCK_DATA* pSock = LockSock(index);
			if (pSock == NULL)
			{
				continue;
			}
			//需要写数据的客户端,才加入KQUEUE检测是否可写
			if (pSock->pSendBuf->NeedWrite())
			{
				kqueue.Operatek(pSock->socker, index, false, kReadEvent | kWriteEvent);
			}
			else
			{
				kqueue.Operatek(pSock->socker, index, false, kReadEvent);
			}
			UnlockSock(index);
		}

		//************************************************************************************
		//处理事件
		int nKqueueEvCount = kqueue.Wait(1);
		if (nKqueueEvCount < 0)
		{
			log_trace_error("[workproc] kqueue.Wait failed:%d", errno);
			return;
		}
		else if (nKqueueEvCount == 0)
		{
			continue;
		}

		struct kevent* kqevents = kqueue.GetOutEvents();
		for (int i = 0; i < nKqueueEvCount; i++)
		{
			struct kevent& ev = kqevents[i];

			int index = (int)(ev.udata);

			if (ev.filter == EVFILT_READ)
			{
				ProcessRead(index, 1);
			}

			if (ev.filter == EVFILT_WRITE)
			{
				ProcessWrite(index, 1);
			}
		}
	}
#endif
}

CDonTcpServer::CDonTcpServer(IDonTcpServerCallBack* callback,int nMaxClients, int nMaxThreads,
	int nSendBufSize, int nRecvBufSize,
	int nSockSendBufSize, int nSockRecvBufSize)
{
	m_nConnectNum = ((nMaxClients <= 0) ? 1 : nMaxClients);
	m_nWorkerNum = ((nMaxThreads <= 0) ? 1 : nMaxThreads);

	m_nSendBufSize = nSendBufSize;
	m_nRecvBufSize = nRecvBufSize;

	m_nSockSendBufSize = nSockSendBufSize;
	m_nSockRecvBufSize = nSockRecvBufSize;

	m_pCallBack = callback;
	m_bQuit = false;
	m_nCount = 0;
	m_nSerial = 0;

	m_pConnects = new CONN_DATA[m_nConnectNum];

	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		m_pConnects[i].pSock = NULL;
	}
}

CDonTcpServer::~CDonTcpServer()
{
	if (m_pCallBack)
	{
		delete m_pCallBack;
		m_pCallBack = NULL;
	}

	//释放运行中资源
	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		SOCK_DATA* p = m_pConnects[i].pSock;

		if (p)
		{
			if (p->pSendBuf) delete p->pSendBuf;
			if (p->pRecvBuf) delete p->pRecvBuf;
			delete p;
		}
	}
	delete[] m_pConnects;


	//释放缓存中资源
	m_pool_sock.Clear();
	m_pool_sendbuf.Clear();
	m_pool_recvbuf.Clear();
}

void CDonTcpServer::SetCallBack(IDonTcpServerCallBack* callback)
{
	m_pCallBack = callback;
}

IDonTcpServerCallBack* CDonTcpServer::GetCallBack() const
{
	return m_pCallBack;
}

bool CDonTcpServer::Initialize(const char* addr, int port)
{
	if (!m_hListenSock.Create())
	{
		log_trace_error("[init][error] create listen socket failed");
		return false;
	}

	if (!m_hListenSock.SetSendBufSize(m_nSockSendBufSize))
	{
		log_trace_error("[init][error] set listen socket send buf size failed");
		return false;
	}

	if (!m_hListenSock.SetRecvBufSize(m_nSockRecvBufSize))
	{
		log_trace_error("[init][error] set listen socket recv buf size failed");
		return false;
	}

	if (!m_hListenSock.Bind(addr, port))
	{
		log_trace_error("[init][error] bind addr:%s port:%d failed", addr, port);
		return false;
	}

	if (!m_hListenSock.Listen(16))
	{
		log_trace_error("[init][error] listen backlog:%d failed", 16);
		return false;
	}

	//创建IOCP/EPOLL/SELECT
	size_t nLeft = m_nConnectNum % m_nWorkerNum;
	size_t nTotalNum = m_nConnectNum + ((nLeft > 0) ? (m_nWorkerNum - nLeft) : 0);
	m_nCapacityNum = nTotalNum / m_nWorkerNum;

#ifdef DON_SOCKET_IOCP
	log_trace_always("[init] create @Riocps");
	m_hListenIocp.Create();
	m_hListenIocp.Operatei(m_hListenSock);
	m_hListenIocp.LoadAcceptEx(m_hListenSock);
	m_hWorkerIocps = new CIocp[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerIocps[i].Create();
	}
#elif DON_SOCKET_EPOLL
	log_trace_always("[init] create @Repolls");
	m_hListenEpoll.Create(1);
	m_hListenEpoll.Operatee(m_hListenSock, m_hListenSock.GetSocket(), EPOLL_CTL_ADD, EPOLLIN);
	m_hWorkerEpolls = new CEpoll[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerEpolls[i].Create(m_nCapacityNum);
	}
#elif DON_SOCKET_SELECT
	log_trace_always("[init] create @Rselects");
	m_hListenSelect.Create(1);
	m_hListenSelect.Operates(m_hListenSock, 0, SELECT_OP_ADD, SELECT_READ);
	m_hWorkerSelects = new CSelect[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerSelects[i].Create(m_nCapacityNum);
	}
#elif DON_SOCKET_KQUEUE
	log_trace_always("[init] create @Rkqueues");
	m_hListenKqueue.Create(1);
	m_hListenKqueue.Operatek(m_hListenSock, m_hListenSock.GetSocket(), false, kReadEvent);
	m_hWorkerKqueues = new CKqueue[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerKqueues[i].Create(m_nCapacityNum);
	}
#endif

	//启动线程
	m_hListenThread = std::thread(&CDonTcpServer::ListenProc, this, this);

	m_hWorkerThreads = new std::thread[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerThreads[i] = std::thread(&CDonTcpServer::WorkerProc, this, this, i);
	}

	log_trace_always("[init] addr:%s port:%d", addr, port);
	log_trace_always("[init] m_nConnectNum:%d m_nWorkerNum:%d m_nCapacityNum:%d", m_nConnectNum, m_nWorkerNum, m_nCapacityNum);
	log_trace_always("[init] server initialized");

	return true;
}

bool CDonTcpServer::Shutdown()
{
	m_bQuit = true;

	log_trace_always("[shut] close listen socket");
	m_hListenSock.Close();

	log_trace_always("[shut] listen thread join");
	m_hListenThread.join();

	log_trace_always("[shut] worker threads join");
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerThreads[i].join();
	}
	delete[] m_hWorkerThreads;

	log_trace_always("[shut] close all clients");
	CloseAll();

#ifdef DON_SOCKET_IOCP
	log_trace_always("[shut] destroy iocps");
	m_hListenIocp.Destory();
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerIocps[i].Destory();
	}
	delete[] m_hWorkerIocps;
#elif DON_SOCKET_EPOLL
	log_trace_always("[shut] destroy epolls");
	m_hListenEpoll.Destory();
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerEpolls[i].Destory();
	}
	delete[] m_hWorkerEpolls;
#elif DON_SOCKET_SELECT
	log_trace_always("[shut] destroy selects");
	m_hListenSelect.Destory();
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerSelects[i].Destory();
	}
	delete[] m_hWorkerSelects;
#elif DON_SOCKET_KQUEUE
	log_trace_always("[shut] destroy kqueues");
	m_hListenKqueue.Destory();
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerKqueues[i].Destory();
	}
	delete[] m_hWorkerKqueues;
#endif

	log_trace_always("[shut] server shutdown");

	return true;
}

bool CDonTcpServer::ClientClose(size_t index, int serial)
{
	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

	if (serial != m_pConnects[index].nSerial)
	{
		UnlockSock(index);
		return false;
	}

	pSock->socker.Close();

	UnlockSock(index);

	return true;
}

bool CDonTcpServer::ClientConnected(size_t index, int serial)
{
	if (index >= m_nConnectNum)
		return false;

	if (serial != m_pConnects[index].nSerial)
	{
		return false;
	}

	return (m_pConnects[index].pSock != NULL);
}

bool CDonTcpServer::ClientSend(size_t index, int serial, const void* pdata, size_t len)
{
	if (index >= m_nConnectNum)
		return false;

	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

	if (serial != m_pConnects[index].nSerial)
	{
		UnlockSock(index);
		return false;
	}
	else if (!pSock->pSendBuf->PushMsg(pdata, len))
	{
		log_trace_error("[send] PushMsg failed,Dump:%s", pSock->pSendBuf->MiniDump().c_str());
		UnlockSock(index);
		return false;
	}

	UnlockSock(index);

	return true;
}

bool CDonTcpServer::ClientSend2(size_t index, int serial,
	const void* pdata1, size_t len1,
	const void* pdata2, size_t len2)
{
	if (index >= m_nConnectNum)
		return false;

	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

	if (serial != m_pConnects[index].nSerial)
	{
		UnlockSock(index);
		return false;
	}
	else if (!pSock->pSendBuf->PushMsg2(pdata1, len1, pdata2, len2))
	{
		log_trace_error("[send] push2 failed,Dump:%s", pSock->pSendBuf->MiniDump().c_str());
		UnlockSock(index);
		return false;
	}

	UnlockSock(index);

	return true;
}

bool CDonTcpServer::Broadcast(const void* pdata, size_t len)
{
	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		if (m_pConnects[i].pSock != NULL)
		{
			if (!ClientSend(i, m_pConnects[i].nSerial, pdata, len))
			{
				log_trace_error("[send] Broadcast failed,index:%d serial:%d", i, m_pConnects[i].nSerial);
			}
		}
	}

	return true;
}

bool CDonTcpServer::Broadcast2(const void* pdata1, size_t len1, const void* pdata2, size_t len2)
{
	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		if (m_pConnects[i].pSock != NULL)
		{
			if (!ClientSend2(i, m_pConnects[i].nSerial, pdata1, len1, pdata2, len2))
			{
				log_trace_error("[send] Broadcast2 failed,index:%d serial:%d", i, m_pConnects[i].nSerial);
			}
		}
	}

	return true;
}

void CDonTcpServer::CloseAll()
{
	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		if (m_pConnects[i].pSock)
		{
			ClientClose(i, m_pConnects[i].nSerial);
		}
	}
}

bool CDonTcpServer::Close(size_t index)
{
	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

	std::string addr = pSock->socker.GetAddr();
	int port = pSock->socker.GetPort();

	pSock->socker.Close();

	DeleteConnect(index);

	UnlockSock(index);

	if (m_pCallBack)
	{
		int serial = m_pConnects[index].nSerial;
		m_pCallBack->OnClose(index, serial, addr.c_str(), port);
	}

	return true;
}

bool CDonTcpServer::Accept(CSocket sock)
{
	if (m_bQuit)
	{
		return false;
	}

	size_t index = NewConnect();

	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		sock.Close();
		return false;
	}

	int serial = m_pConnects[index].nSerial;

	pSock->socker = sock;

#ifdef DON_SOCKET_IOCP
	if (!m_hWorkerIocps[pSock->nWorkerId].Operatei(sock, index))
	{
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_EPOLL
	if (!m_hWorkerEpolls[pSock->nWorkerId].Operatee(sock, index, EPOLL_CTL_ADD, EPOLLIN))
	{
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_SELECT
	if (!m_hWorkerSelects[pSock->nWorkerId].Operates(sock, index, SELECT_OP_ADD, SELECT_READ))
	{
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_KQUEUE
	if (!m_hWorkerKqueues[pSock->nWorkerId].Operatek(sock, index, false, kReadEvent))
	{
		UnlockSock(index);
		Close(index);
		return false;
	}
#endif

	UnlockSock(index);

	if (m_pCallBack)
	{
		m_pCallBack->OnAccept(index, serial, sock.GetAddr().c_str(), sock.GetPort());
	}

	return true;
}

long CDonTcpServer::GetSerial()
{
	long res = ++m_nSerial;

	if (res == 0)
	{
		res = ++m_nSerial;
	}

	return res;
}

int CDonTcpServer::NewConnect()
{
	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		if (m_pConnects[i].pSock != NULL)
		{
			continue;
		}

		if (m_pConnects[i].cdMutex.try_lock())
		{
			if (NULL == m_pConnects[i].pSock)
			{
				SOCK_DATA* pSock = m_pool_sock.New(); //new SOCK_DATA;
				::memset(pSock, 0, sizeof(pSock));

				pSock->pSendBuf = m_pool_sendbuf.New();
				pSock->pRecvBuf = m_pool_recvbuf.New();
				pSock->pSendBuf->ReNew(m_nSendBufSize);//重新初始化CBuffer
				pSock->pRecvBuf->ReNew(m_nRecvBufSize);//重新初始化CBuffer

				pSock->nWorkerId = i / m_nCapacityNum;

				m_pConnects[i].pSock = pSock;
				m_pConnects[i].nSerial = GetSerial();

				m_nCount++;

				m_pConnects[i].cdMutex.unlock();

				return int(i);
			}

			m_pConnects[i].cdMutex.unlock();
		}
	}

	return int(m_nConnectNum);
}

void CDonTcpServer::DeleteConnect(size_t index)
{
	if (index >= m_nConnectNum)
		return;

	SOCK_DATA* pSock = m_pConnects[index].pSock;

	if (pSock)
	{
		if (pSock->pSendBuf) m_pool_sendbuf.Delete(pSock->pSendBuf);
		if (pSock->pRecvBuf) m_pool_recvbuf.Delete(pSock->pRecvBuf);

		m_pool_sock.Delete(pSock);
	}

	m_pConnects[index].pSock = NULL;

	m_nCount--;
}

CDonTcpServer::SOCK_DATA* CDonTcpServer::LockSock(size_t index)
{
	if (index >= m_nConnectNum)
	{
		return NULL;
	}

	m_pConnects[index].cdMutex.lock();

	if (NULL == m_pConnects[index].pSock)
	{
		m_pConnects[index].cdMutex.unlock();

		return NULL;
	}

	return m_pConnects[index].pSock;
}

CDonTcpServer::SOCK_DATA* CDonTcpServer::GetSock(size_t index)
{
	if (index >= m_nConnectNum)
	{
		return NULL;
	}

	if (NULL == m_pConnects[index].pSock)
	{
		return NULL;
	}

	return m_pConnects[index].pSock;
}

void CDonTcpServer::UnlockSock(size_t index)
{
	if (index >= m_nConnectNum)
		return;

	m_pConnects[index].cdMutex.unlock();
}

bool CDonTcpServer::ProcessWrite(size_t index, size_t size)
{
	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

#ifdef DON_SOCKET_IOCP
	if (!pSock->pSendBuf->WriteIocp(size))
	{
		log_trace_error("[write] WriteIocp error,close,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_EPOLL
	if (!pSock->pSendBuf->WriteSocket(pSock->socker))
	{
		log_trace_error("[write] WriteSocket error,close,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_SELECT
	if (!pSock->pSendBuf->WriteSocket(pSock->socker))
	{
		log_trace_error("[write] WriteSocket error,close,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_KQUEUE
	if (!pSock->pSendBuf->WriteSocket(pSock->socker))
	{
		log_trace_error("[write] WriteSocket error,close,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#endif

	UnlockSock(index);

	return true;
}

bool CDonTcpServer::ProcessRead(size_t index, size_t size)
{
	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

#ifdef DON_SOCKET_IOCP
	if (!pSock->pRecvBuf->ReadIocp(size))
	{
		log_trace_error("[read] ReadIocp error,close,Dump:%s", pSock->pRecvBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_EPOLL
	if (!pSock->pRecvBuf->ReadSocket(pSock->socker))
	{
		log_trace_error("[read] ReadSocket error,close,Dump:%s", pSock->pRecvBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_SELECT
	if (!pSock->pRecvBuf->ReadSocket(pSock->socker))
	{
		log_trace_error("[read] ReadSocket error,close,Dump:%s", pSock->pRecvBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_KQUEUE
	if (!pSock->pRecvBuf->ReadSocket(pSock->socker))
	{
		log_trace_error("[read] ReadSocket error,close,Dump:%s", pSock->pRecvBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#endif
	int serial = m_pConnects[index].nSerial;

	//处理所有消息
	while (true)
	{
		char msg_buf[1024];
		int msg_len = 0;

		msg_len = pSock->pRecvBuf->PopMsg(msg_buf, sizeof(msg_buf));
		if (msg_len == BUFFER_WAITING)
		{
			log_trace_error("[read] ProcessRead PopMsg error,msg is too short,do nothing");
			UnlockSock(index);
			return false;
		}
		if (msg_len == BUFFER_ERROR)
		{
			log_trace_error("[read] ProcessRead PopMsg error,msg is too long,discard");
			UnlockSock(index);
			return false;
		}
		if (msg_len == BUFFER_EMPTY)
		{
			//log_trace_error("[read] ProcessRead PopMsg error,buffer is empty,return");
			UnlockSock(index);
			return false;
		}

		if (m_pCallBack && msg_len)
		{
			m_pCallBack->OnReceive(index, serial, msg_buf, msg_len);
		}
	}

	UnlockSock(index);

	return true;
}

std::string CDonTcpServer::Dump(size_t index)
{
	std::string strDump;

	SOCK_DATA* pSock = GetSock(index);

	if (NULL == pSock)
	{
		return "[Dump:ERROR]";
	}

	std::string strSendBuf = pSock->pSendBuf->MiniDump();
	std::string strRecvBuf = pSock->pRecvBuf->MiniDump();
	strDump = "[S]" + strSendBuf + "[R]" + strRecvBuf;

	return strDump;
}

