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

#include"Iocp.h"

#ifdef DON_SOCKET_IOCP

CIocp::~CIocp()
{
	Destory();
}

bool CIocp::Create()
{
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hCompletionPort)
	{
		//log
		return false;
	}
	return true;
}

void CIocp::Destory()
{
	if (m_hCompletionPort)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;
	}
}

//关联IOCP与sockfd
bool CIocp::Operatei(CSocket sockfd)
{
	//完成键
	auto ret = CreateIoCompletionPort((HANDLE)sockfd.GetSocket(), m_hCompletionPort, (ULONG_PTR)sockfd.GetSocket(), 0);
	if (!ret)
	{
		//log
		return false;
	}
	return true;
}

//关联IOCP与自定义数据地址
bool CIocp::Operatei(CSocket sockfd, void* pVoid)
{
	//完成键
	auto ret = CreateIoCompletionPort((HANDLE)sockfd.GetSocket(), m_hCompletionPort, (ULONG_PTR)pVoid, 0);
	if (!ret)
	{
		//log
		return false;
	}
	return true;
}

//关联IOCP与自定义数据地址
bool CIocp::Operatei(CSocket sockfd, size_t index)
{
	//完成键
	auto ret = CreateIoCompletionPort((HANDLE)sockfd.GetSocket(), m_hCompletionPort, (ULONG_PTR)index, 0);
	if (!ret)
	{
		//log
		return false;
	}
	return true;
}
//向IOCP投递接受连接的任务
bool CIocp::PostAccept(IOCP_DATA_BASE* pIO_DATA)
{
	//1. accept、WSAAccept是同步操作，AcceptEx是异步操作
	//2. WSAAccept函数早accept函数基础上添加了条件函数判断是否接受客户端连接
	//3. AcceptEx是异步的，可以同时发出多个AcceptEx请求，支持重叠IO操作.
	if (!m_AcceptEx)
	{
		//log
		return false;
	}

	pIO_DATA->iotype = IOCP_EVENT_ACCEPT;
	pIO_DATA->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (FALSE == m_AcceptEx(m_sockServer.GetSocket()
		, pIO_DATA->sockfd
		, pIO_DATA->wsabuff.buf
		, 0
		, sizeof(sockaddr_in) + 16
		, sizeof(sockaddr_in) + 16
		, NULL
		, &pIO_DATA->overlapped
	))
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			//log
			return false;
		}
	}
	return true;
}

//向IOCP投递接收数据的任务
bool CIocp::PostReceive(IOCP_DATA_BASE* pIO_DATA)
{
	pIO_DATA->iotype = IOCP_EVENT_RECV;
	DWORD flags = 0;
	ZeroMemory(&pIO_DATA->overlapped, sizeof(OVERLAPPED));

	if (SOCKET_ERROR == WSARecv(pIO_DATA->sockfd, &pIO_DATA->wsabuff, 1, NULL, &flags, &pIO_DATA->overlapped, NULL))
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			if (WSAECONNRESET == err)
			{
				return false;
			}

			//log 
			return false;
		}
	}
	return true;
}
//向IOCP投递发送数据的任务
bool CIocp::PostSend(IOCP_DATA_BASE* pIO_DATA)
{
	pIO_DATA->iotype = IOCP_EVENT_SEND;
	DWORD flags = 0;
	ZeroMemory(&pIO_DATA->overlapped, sizeof(OVERLAPPED));

	if (SOCKET_ERROR == WSASend(pIO_DATA->sockfd, &pIO_DATA->wsabuff, 1, NULL, flags, &pIO_DATA->overlapped, NULL))
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			if (WSAECONNRESET == err)
			{
				return false;
			}

			//log
			return false;
		}
	}
	return true;
}

//ret = -1 iocp出错
//ret =  0 没有事件
//ret =  1 有事件发生
int CIocp::Wait(int timeout_ms)
{
	m_ioEvent = {};
	m_ioEvent.bytesTrans = 0;
	m_ioEvent.pIoData = NULL;
	m_ioEvent.data.pVoid = NULL;

	if (FALSE == GetQueuedCompletionStatus(m_hCompletionPort
		, &m_ioEvent.bytesTrans
		, (PULONG_PTR)&m_ioEvent.data
		, (LPOVERLAPPED*)&m_ioEvent.pIoData
		, timeout_ms))
	{
		int err = GetLastError();
		if (WAIT_TIMEOUT == err)
		{
			return 0;
		}
		if (ERROR_NETNAME_DELETED == err)
		{
			return 1;
		}
		if (ERROR_CONNECTION_ABORTED == err)
		{
			return 1;
		}

		//printf("GetQueuedCompletionStatus failed\n");
		return -1;
	}
	return 1;
}

bool CIocp::LoadAcceptEx(CSocket ListenSocket)
{
	if (m_sockServer.IsValid())
	{
		//log
		return false;
	}
	if (m_AcceptEx)
	{
		//printf("LoadAcceptEx m_AcceptEx != NULL\n");
		return false;
	}
	m_sockServer = ListenSocket;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	int iResult = WSAIoctl(ListenSocket.GetSocket(), SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&m_AcceptEx, sizeof(m_AcceptEx),
		&dwBytes, NULL, NULL);

	if (iResult == SOCKET_ERROR)
	{
		//printf("WSAIoctl failed with error: %u\n", WSAGetLastError());
		return false;
	}
	return true;
}

#endif //DON_SOCKET_IOCP
