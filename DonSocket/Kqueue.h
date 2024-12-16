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

#ifndef KQUEUE_H
#define KQUEUE_H

#include "Buffer.h"

#ifdef DON_SOCKET_KQUEUE

const int kReadEvent = 1;
const int kWriteEvent = 2;

class CKqueue
{
public:
	~CKqueue();

public:
	bool Create(int nMaxEvents);
	void Destory();
	int	 Wait(int timeout_ms);
	bool Operatek(CSocket& sockfd, size_t index, bool modify, int events);
public:
	struct kevent*	GetOutEvents()	{	return m_pEvents;}

private:
	struct kevent*	m_pEvents = NULL;
	int		m_nMaxEvents = 1;
	int		m_kqfd = -1;
};

#endif //DON_SOCKET_KQUEUE


#endif // KQUEUE_H
