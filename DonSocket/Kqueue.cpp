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

#include "Kqueue.h"

#ifdef DON_SOCKET_KQUEUE

CKqueue::~CKqueue()
{
	Destory();
}

bool CKqueue::Create(int nMaxEvents)
{
	if (m_kqfd > 0)
	{
		Destory();
	}

	m_kqfd = kqueue();
	if ( m_kqfd < 0 )
	{
		return false;
	}

	nMaxEvents = nMaxEvents * 2; //kevent与epoll最大的不同在于READ/WRITE事件是分开注册并且分开返回的，而Epoll则是一个fd一次返回读和写事件，用标志位来判断

	m_pEvents = new struct kevent[nMaxEvents];
	m_nMaxEvents = nMaxEvents;

	return true;
}

void CKqueue::Destory()
{
	if (m_kqfd > 0)
	{
		close(m_kqfd);
		m_kqfd = -1;
	}

	if (m_pEvents)
	{
		delete[] m_pEvents;
		m_pEvents = NULL;
	}
}

bool CKqueue::Operatek(CSocket& sockfd, size_t index, bool modify, int events)
{
	int fd = sockfd.GetSocket();

	struct kevent ev[2];
	int n = 0;
	if (events & kReadEvent) 
	{
		EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)(intptr_t)index);
	}
	else if (modify) 
	{
		EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, (void*)(intptr_t)index);
	}
	
	if (events & kWriteEvent) 
	{
		EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)(intptr_t)index);
	}
	else if (modify) 
	{
		EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, (void*)(intptr_t)index);
	}

	/*
	printf("%s fd %d events read %d write %d\n",
		modify ? "mod" : "add", fd, events & kReadEvent, events & kWriteEvent);
	*/

	int ret = kevent(m_kqfd, ev, n, NULL, 0, NULL);
	if (ret != 0)
	{
		return false;
	}
	
	return true;
}

int CKqueue::Wait(int timeout_ms)
{
	struct timespec timeout;
	timeout.tv_sec = timeout_ms / 1000;
	timeout.tv_nsec = (timeout_ms % 1000) * 1000 * 1000;
	
	int ret = kevent(m_kqfd, NULL, 0, m_pEvents, m_nMaxEvents, &timeout);
	/*
	if (EPOLL_ERROR == ret)
	{
		if (errno == EINTR)
		{
			return 0;
		}
	}*/

	return ret;
}

#endif //DON_SOCKET_KQUEUE

