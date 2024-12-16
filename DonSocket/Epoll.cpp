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

#include "Epoll.h"

#ifdef DON_SOCKET_EPOLL

CEpoll::~CEpoll()
{
	Destory();
}

bool CEpoll::Create(int nMaxEvents)
{
	if (m_epfd > 0)
	{
		Destory();
	}

	m_epfd = epoll_create(nMaxEvents);
	if (EPOLL_ERROR == m_epfd)
	{
		return false;
	}
	m_pEvents = new epoll_event[nMaxEvents];
	m_nMaxEvents = nMaxEvents;

	return true;
}

void CEpoll::Destory()
{
	if (m_epfd > 0)
	{
		close(m_epfd);
		m_epfd = -1;
	}

	if (m_pEvents)
	{
		delete[] m_pEvents;
		m_pEvents = NULL;
	}
}

bool CEpoll::Operatee(CSocket& sockfd, size_t index, int op, uint32_t events)
{
	epoll_event ev;
	ev.events = events;
	ev.data.fd = index;
	
	//返回0代表操作成功，返回负值代表失败 -1
	int ret = epoll_ctl(m_epfd, op, sockfd.GetSocket(), &ev);
	if (EPOLL_ERROR == ret)
	{
		return false;
	}
	return true;
}

int CEpoll::Wait(int timeout)
{
	//m_epfd epoll对象的描述符
	//m_pEvents 用于接收检测到的网络事件的数组
	//maxevents 接收数组的大小，能够接收的事件数量
	//timeout
	//t=-1 直到有事件发生才返回
	//t= 0 立即返回
	//t> 0 如果没有事件那么等待t毫秒后返回。
	int ret = epoll_wait(m_epfd, m_pEvents, m_nMaxEvents, timeout);
	if (EPOLL_ERROR == ret)
	{
		if (errno == EINTR)
		{
			return 0;
		}
	}
	return ret;
}

#endif //DON_SOCKET_EPOLL
