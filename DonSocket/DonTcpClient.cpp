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

#include "DonTcpClient.h"
#include "../Util/Util.h"

void CDonTcpClient::ConnectProc(void* lpParameter)
{
	CDonTcpClient* pthis = (CDonTcpClient*)lpParameter;

	while (!pthis->m_bQuit)
	{
		for (size_t i = 0; i < pthis->m_nConnectNum; i++)
		{
			if (pthis->m_Start && (!pthis->m_bQuit))
			{
				pthis->CheckConnect(i);
			}
		}

		DonSleep(1000);
	}
}

void CDonTcpClient::WorkerProc(void* lpParameter, size_t nThreadId)
{
	CDonTcpClient* pthis = (CDonTcpClient*)lpParameter;
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
			//需要写数据的客户端,才加入SELECT_WRITE检测是否可写
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

CDonTcpClient::CDonTcpClient(IDonTcpClientCallBack* callback,int nMaxConnects, int nMaxThreads,
	int nSendBufSize, int nRecvBufSize,
	int nSockSendBufSize, int nSockRecvBufSize)
{
	m_nConnectNum = ((nMaxConnects <= 0) ? 1 : nMaxConnects);
	m_nWorkerNum = ((nMaxThreads <= 0) ? 1 : nMaxThreads);

	m_nSendBufSize = nSendBufSize;
	m_nRecvBufSize = nRecvBufSize;

	m_nSockSendBufSize = nSockSendBufSize;
	m_nSockRecvBufSize = nSockRecvBufSize;

	m_bQuit = false;
	m_Start = 0;
	m_nCount = 0;

	m_nDestPort = 0;
	m_pCallBack = callback;
	m_pConnects = new CONN_DATA[m_nConnectNum];

	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		m_pConnects[i].nState = 0;
		m_pConnects[i].nCount = 0;
		m_pConnects[i].pSock = NULL;
	}
}

CDonTcpClient::~CDonTcpClient()
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

void CDonTcpClient::SetCallBack(IDonTcpClientCallBack* callback)
{
	m_pCallBack = callback;
}

IDonTcpClientCallBack* CDonTcpClient::GetCallBack() const
{
	return m_pCallBack;
}

bool CDonTcpClient::Initialize(const char* addr, int port, bool bStart)
{
	m_strDestAddr = addr;
	m_nDestPort = port;

	m_bQuit = false;
	m_Start = bStart;

	//创建IOCP/EPOLL/SELECT
	size_t nLeft = m_nConnectNum % m_nWorkerNum;
	size_t	nTotalNum = m_nConnectNum + ((nLeft > 0) ? (m_nWorkerNum - nLeft) : 0);
	m_nCapacityNum = nTotalNum / m_nWorkerNum;

#ifdef DON_SOCKET_IOCP
	log_trace_always("[init] create @Riocps");
	m_hWorkerIocps = new CIocp[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerIocps[i].Create();
	}
#elif DON_SOCKET_EPOLL
	log_trace_always("[init] create @Repolls");
	m_hWorkerEpolls = new CEpoll[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerEpolls[i].Create(m_nCapacityNum);
	}
#elif DON_SOCKET_SELECT
	log_trace_always("[init] create @Rselects");
	m_hWorkerSelects = new CSelect[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerSelects[i].Create(m_nCapacityNum);
	}
#elif DON_SOCKET_KQUEUE
	log_trace_always("[init] create @Rkqueues");
	m_hWorkerKqueues = new CKqueue[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerKqueues[i].Create(m_nCapacityNum);
	}
#endif

	//启动线程
	m_hConnectThread = std::thread(&CDonTcpClient::ConnectProc, this, this);
	m_hWorkerThreads = new std::thread[m_nWorkerNum];
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerThreads[i] = std::thread(&CDonTcpClient::WorkerProc, this, this, i);
	}

	log_trace_always("[init] addr:%s port:%d", addr, port);
	log_trace_always("[init] m_nConnectNum:%d m_nWorkerNum:%d m_nCapacityNum:%d", m_nConnectNum, m_nWorkerNum, m_nCapacityNum);
	log_trace_always("[init] client initialized");

	return true;
}

bool CDonTcpClient::Shutdown()
{
	log_trace_always("[shut] stop check connect thread");
	Stop();

	m_bQuit = true;
	log_trace_always("[shut] connect thread join");
	m_hConnectThread.join();

	log_trace_always("[shut] worker threads join");
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerThreads[i].join();
	}
	delete[] m_hWorkerThreads;

	log_trace_always("[shut] close all connections");
	CloseAll();

#ifdef DON_SOCKET_IOCP
	log_trace_always("[shut] destroy iocps");
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerIocps[i].Destory();
	}
	delete[] m_hWorkerIocps;
#elif DON_SOCKET_EPOLL
	log_trace_always("[shut] destroy epolls");
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerEpolls[i].Destory();
	}
	delete[] m_hWorkerEpolls;
#elif DON_SOCKET_SELECT
	log_trace_always("[shut] destroy selects");
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerSelects[i].Destory();
	}
	delete[] m_hWorkerSelects;
#elif DON_SOCKET_KQUEUE
	log_trace_always("[shut] destroy kqueues");
	for (size_t i = 0; i < m_nWorkerNum; i++)
	{
		m_hWorkerKqueues[i].Destory();
	}
	delete[] m_hWorkerKqueues;
#endif

	log_trace_always("[shut] client shutdown");

	return true;
}

bool CDonTcpClient::ClientClose(size_t index)
{
	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

	pSock->socker.Close();

	UnlockSock(index);

	return true;
}

bool CDonTcpClient::ClientConnected(size_t index)
{
	if (index >= m_nConnectNum)
		return false;

	return (m_pConnects[index].nState == SS_CONNECTED);
}

bool CDonTcpClient::ClientSend(size_t index, const void* pdata, size_t len)
{
	if (index >= m_nConnectNum)
		return false;

	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

	if (!pSock->pSendBuf->PushMsg(pdata, len))
	{
		log_trace_error("[send] PushMsg failed,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		return false;
	}

	UnlockSock(index);

	return true;
}

bool CDonTcpClient::ClientSend2(size_t index, const void* pdata1, size_t len1, const void* pdata2, size_t len2)
{
	if (index >= m_nConnectNum)
		return false;

	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

	if (!pSock->pSendBuf->PushMsg2(pdata1, len1, pdata2, len2))
	{
		log_trace_error("[send] push2 failed,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		return false;
	}

	UnlockSock(index);

	return true;
}

bool CDonTcpClient::ClientSendAll(const void* pdata, size_t len)
{
	size_t sendNum = 0;
	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		if (m_pConnects[i].pSock)
		{
			if (!ClientSend(i, pdata, len))
			{
				log_trace_error("[send] ClientSendAll failed. index:%d", i);
			}
			else
			{
				sendNum++;
			}
		}
	}

	if (m_nConnectNum <= 0 || sendNum != m_nConnectNum)
		return false;

	return true;
}

bool CDonTcpClient::ClientSendAll2(const void* pdata1, size_t len1, const void* pdata2, size_t len2)
{
	size_t sendNum = 0;
	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		if (m_pConnects[i].pSock)
		{
			if (!ClientSend2(i, pdata1, len1, pdata2, len2))
			{
				log_trace_error("[send] ClientSendAll2 failed. index:%d", i);
			}
			else
			{
				sendNum++;
			}
		}
	}

	if (m_nConnectNum <= 0 || sendNum != m_nConnectNum)
		return false;

	return true;
}

void CDonTcpClient::CloseAll()
{
	for (size_t i = 0; i < m_nConnectNum; i++)
	{
		if (m_pConnects[i].pSock)
		{
			ClientClose(i);
		}
	}
}

bool CDonTcpClient::IsStart() const
{
	return (m_Start == 1);
}

bool CDonTcpClient::Start()
{
	if (IsStart())
	{
		return false;
	}

	m_Start = 1;

	return true;
}

bool CDonTcpClient::Stop()
{
	m_Start = 0;

	return true;
}

void CDonTcpClient::CheckConnect(size_t index)
{
	if (index >= m_nConnectNum)
		return;

	if (m_pConnects[index].nState == SS_CONNECTED)
	{
		return;
	}

	bool bConnected = false;
	bool bError = false;
	std::string strAddr;
	size_t nPort;

	m_pConnects[index].cdMutex.lock();

	switch (m_pConnects[index].nState)
	{
	case SS_INIT:
	{
		CSocket sock;
		if (!sock.Create())
		{
			SwitchState(index, SS_FAILED);
			break;
		}

		if (!sock.SetSendBufSize(m_nSockSendBufSize))
		{
			sock.Close();
			SwitchState(index, SS_FAILED);
			break;
		}

		if (!sock.SetRecvBufSize(m_nSockRecvBufSize))
		{
			sock.Close();
			SwitchState(index, SS_FAILED);
			break;
		}

		if (!sock.Connect(m_strDestAddr.c_str(), m_nDestPort))
		{
			sock.Close();
			SwitchState(index, SS_FAILED);
			break;
		}

		SOCK_DATA* pSock = NewConnect(index);
		pSock->socker = sock;

#ifdef DON_SOCKET_IOCP
		if (!m_hWorkerIocps[pSock->nWorkerId].Operatei(sock, index))
#elif DON_SOCKET_EPOLL
		if (!m_hWorkerEpolls[pSock->nWorkerId].Operatee(sock, index, EPOLL_CTL_ADD, EPOLLIN))
#elif DON_SOCKET_SELECT
		if (!m_hWorkerSelects[pSock->nWorkerId].Operates(sock, index, SELECT_OP_ADD, SELECT_READ))
#elif DON_SOCKET_KQUEUE
		if (!m_hWorkerKqueues[pSock->nWorkerId].Operatek(sock, index, false, kReadEvent))
#endif
		{
			log_trace_error("CheckConnect,Operate failed");
			SwitchState(index, SS_FAILED);

			bError = true;
		}
		else
		{
			SwitchState(index, SS_CONNECTED);

			bConnected = true;
			strAddr = sock.GetAddr();
			nPort = sock.GetPort();
		}
	}
	break;
	case SS_CONNECTED:
	{
	}
	break;
	case SS_FAILED:
	{
		SwitchState(index, SS_INIT);
	}
	break;
	default:
		break;
	}

	m_pConnects[index].cdMutex.unlock();

	if (bError)
	{
		Close(index);
	}

	if (m_pCallBack && bConnected)
	{
		m_pCallBack->OnConnect(index, strAddr.c_str(), nPort);
	}
}

void CDonTcpClient::SwitchState(size_t index, int state)
{
	if (index >= m_nConnectNum)
		return;

	m_pConnects[index].nState = state;
	m_pConnects[index].nCount = 0;
}

int CDonTcpClient::GetState(size_t index)
{
	if (index >= m_nConnectNum)
		return SS_FAILED;;

	return m_pConnects[index].nState;
}

CDonTcpClient::SOCK_DATA* CDonTcpClient::NewConnect(size_t index)
{
	SOCK_DATA* pSock = m_pool_sock.New();
	::memset(pSock, 0, sizeof(pSock));

	//pSock->socker = sock;
	pSock->pSendBuf = m_pool_sendbuf.New();
	pSock->pRecvBuf = m_pool_recvbuf.New();
	pSock->pSendBuf->ReNew(m_nSendBufSize);//重新初始化CBuffer
	pSock->pRecvBuf->ReNew(m_nRecvBufSize);//重新初始化CBuffer

	pSock->nWorkerId = index / m_nCapacityNum;

	m_pConnects[index].pSock = pSock;

	m_nCount++;

	return pSock;
}

void CDonTcpClient::DeleteConnect(size_t index)
{
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

bool CDonTcpClient::Close(size_t index)
{
	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

	if (GetState(index) == SS_FAILED)
	{
		UnlockSock(index);
		return false;
	}

	std::string addr = pSock->socker.GetAddr().c_str();
	int port = pSock->socker.GetPort();

	pSock->socker.Close();

	DeleteConnect(index);

	SwitchState(index, SS_FAILED);

	UnlockSock(index);

	if (m_pCallBack)
	{
		m_pCallBack->OnClose(index, addr.c_str(), port);
	}

	return true;
}

CDonTcpClient::SOCK_DATA* CDonTcpClient::LockSock(size_t index)
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

CDonTcpClient::SOCK_DATA* CDonTcpClient::GetSock(size_t index)
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

void CDonTcpClient::UnlockSock(size_t index)
{
	if (index >= m_nConnectNum)
		return;

	m_pConnects[index].cdMutex.unlock();
}

bool CDonTcpClient::ProcessWrite(size_t index, size_t size)
{
	SOCK_DATA* pSock = LockSock(index);

	if (NULL == pSock)
	{
		return false;
	}

#ifdef DON_SOCKET_IOCP
	if (!pSock->pSendBuf->WriteIocp(size))
	{
		log_trace_error("[write] WriteIocp error,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_EPOLL
	if (!pSock->pSendBuf->WriteSocket(pSock->socker))
	{
		log_trace_error("[write] WriteSocket error,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_SELECT
	if (!pSock->pSendBuf->WriteSocket(pSock->socker))
	{
		log_trace_error("[write] WriteSocket error,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#elif DON_SOCKET_KQUEUE
	if (!pSock->pSendBuf->WriteSocket(pSock->socker))
	{
		log_trace_error("[write] WriteSocket error,Dump:%s", pSock->pSendBuf->Dump().c_str());
		UnlockSock(index);
		Close(index);
		return false;
	}
#endif
	UnlockSock(index);

	return true;
}

bool CDonTcpClient::ProcessRead(size_t index, size_t size)
{
	// 临时接收缓存，接收后复制消息到这里，使得回调中能发送消息
	char msg_buf[1024];
	int msg_len = 0;

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

	//只处理一条消息
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
		log_trace_error("[read] ProcessRead PopMsg error,buffer is empty,return");
		UnlockSock(index);
		return false;
	}
	UnlockSock(index);

	if (m_pCallBack && msg_len)
	{
		m_pCallBack->OnReceive(index, msg_buf, msg_len);
	}

	return true;
}

std::string CDonTcpClient::Dump(size_t index)
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

