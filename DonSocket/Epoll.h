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

#ifndef EPOLL_H
#define EPOLL_H

#include "Buffer.h"

#ifdef DON_SOCKET_EPOLL

class CEpoll
{
public:
	~CEpoll();

public:
	bool Create(int nMaxEvents);
	void Destory();
	int	 Wait(int timeout);
	bool Operatee(CSocket& sockfd, size_t index, int op, uint32_t events);
	
public:
	epoll_event*	GetOutEvents()	{	return m_pEvents;}

private:
	epoll_event*	m_pEvents = NULL;
	int				m_nMaxEvents = 1;
	int				m_epfd = -1;
};

#endif //DON_SOCKET_EPOLL


#endif // EPOLL_H
