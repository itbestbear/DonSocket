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

#include"Select.h"

#ifdef DON_SOCKET_SELECT

CFdSet::CFdSet(int nMaxFdCount)
{
#ifdef _WIN32
	nMaxFdCount = nMaxFdCount + 1;	//预留一个空间，以防fd_set结构在不同的字节对齐模式下，u_int fd_count所占字节大小不同
	m_nfdSize = sizeof(u_int) + (sizeof(SOCKET) * nMaxFdCount);
#else
	nMaxFdCount = MAX_FD_COUNT;		//linux无法优化空间占用
	m_nfdSize = nMaxFdCount / (8 * sizeof(char));
#endif // _WIN32
	m_pfdset = (fd_set*)new char[m_nfdSize];
	memset(m_pfdset, 0, m_nfdSize);
}

CFdSet::~CFdSet()
{
	if (m_pfdset)
	{
		delete[] m_pfdset;
		m_pfdset = nullptr;
	}
}

void CFdSet::add(SOCKET s)
{
	if (!FD_ISSET(s, m_pfdset))
	{
		FD_SET(s, m_pfdset);

		m_nfdcount += 1;
	}
}

void CFdSet::del(SOCKET s)
{
	if (FD_ISSET(s, m_pfdset))
	{
		FD_CLR(s, m_pfdset);

		m_nfdcount -= 1;
	}
}

void CFdSet::zero()
{
#ifdef _WIN32
	FD_ZERO(m_pfdset);
#else
	memset(m_pfdset, 0, m_nfdSize);
#endif // _WIN32

	m_nfdcount = 0;
}

bool CFdSet::has(SOCKET s)
{
	return FD_ISSET(s, m_pfdset);
}

fd_set* CFdSet::fdset()
{
	return m_pfdset;
}

void CFdSet::copy(CFdSet& set)
{
	memcpy(m_pfdset, set.fdset(), set.m_nfdSize);
	m_nfdcount = set.FdCount();
}

CSelect::~CSelect()
{
	Destory();
}

bool CSelect::Create(int nMaxEvents)
{
	m_pfdRead  = new CFdSet(nMaxEvents);
	m_pfdWrite = new CFdSet(nMaxEvents);

	m_pRegEvents = new SELECT_EVENT[nMaxEvents];
	m_pOutEvents = new SELECT_EVENT[nMaxEvents];
	m_nMaxCount = nMaxEvents;

	return true;
}

void CSelect::Destory()
{
	if (m_pfdRead)  { delete m_pfdRead;  m_pfdRead = NULL;}
	if (m_pfdWrite) { delete m_pfdWrite; m_pfdWrite = NULL; }

	if (m_pRegEvents)
	{
		delete[] m_pRegEvents;
		m_pRegEvents = NULL;
	}

	if (m_pOutEvents)
	{
		delete[] m_pOutEvents;
		m_pOutEvents = NULL;
	}
}

bool CSelect::Operates(CSocket sockfd, size_t index, int op, int type)
{
	std::lock_guard<std::mutex> lock(m_mtEvent);

	switch (op)
	{
	case SELECT_OP_ADD:
		return Add(sockfd, index, type);
	case SELECT_OP_MOD:
		return Modify(sockfd, index, type);
	case SELECT_OP_DEL:
		return Delete(sockfd, index);
	default:
		break;
	}
	return false;
}

bool CSelect::Add(CSocket sockfd, size_t index, int type)
{
	SOCKET fd = sockfd.GetSocket();

	if (type & SELECT_READ)
	{
		if (m_pfdRead->FdCount() >= m_nMaxCount)
			return false;

		m_pfdRead->add(fd);
	}
		
	if (type & SELECT_WRITE)
	{
		if (m_pfdWrite->FdCount() >= m_nMaxCount)
			return false;

		m_pfdWrite->add(fd);
	}
		
	if (fd > m_maxSock)
		m_maxSock = fd;
	
	return true;
}

bool CSelect::Modify(CSocket sockfd, size_t index, int type)
{
	size_t nLocalIndex = index - m_nBaseIndex;
	if (nLocalIndex >= m_nMaxCount)
	{
		//log
		return false;
	}

	SOCKET fd = sockfd.GetSocket();

	bool isadd = false;
	if (type & SELECT_READ)
	{
		m_pfdRead->add(fd);
		isadd = true;
	}
	else
	{
		m_pfdRead->del(fd);
	}
		
	if (type & SELECT_WRITE)
	{
		m_pfdWrite->add(fd);
		isadd = true;
	}
	else
	{
		m_pfdWrite->del(fd);
	}

	SELECT_EVENT& event = m_pRegEvents[nLocalIndex];

	event.sockfd = ( isadd ? fd : 0 );
	event.data.index = ( isadd ? index : 0 );
	event.eventType = SELECT_NONE;//发生的事件，select后赋值

	return true;
}

bool CSelect::Delete(CSocket sockfd, size_t index)
{
	size_t nLocalIndex = index - m_nBaseIndex;
	if (nLocalIndex >= m_nMaxCount)
	{
		//log
		return false;
	}

	SELECT_EVENT& event = m_pRegEvents[nLocalIndex];

	SOCKET fd = sockfd.GetSocket();
	event.sockfd = 0;
	event.data.index = 0;
	event.eventType = SELECT_NONE;//发生的事件，select后赋值

	m_pfdRead->del(fd);
	m_pfdWrite->del(fd);

	return true;
}

void CSelect::Zero()
{
	m_pfdRead->zero();
	m_pfdWrite->zero();

	for (int i = 0; i < m_nMaxCount; i++)
	{
		m_pRegEvents[i].eventType = 0;
		m_pRegEvents[i].sockfd=0;
		m_pRegEvents[i].data.index = 0;
	}
}

int CSelect::Wait(int timeout_ms)
{
	//清空发生事件
	int nOutCount = 0;

	std::lock_guard<std::mutex> lock(m_mtEvent);

	timeval t{ 0,timeout_ms };
	int ret = select(m_maxSock + 1, 
		m_pfdRead->IsEmpty() ? NULL : m_pfdRead->fdset(),
		m_pfdWrite->IsEmpty() ? NULL : m_pfdWrite->fdset(),
		NULL,
		&t);

	for (int i = 0; i < m_nMaxCount; i++)
	{
		SELECT_EVENT& event = m_pRegEvents[i];

		if (event.sockfd == 0)
			continue;

		SOCKET fd = event.sockfd;

		int type = SELECT_NONE;
		if (m_pfdRead->has(fd))
			type |= SELECT_READ;

		if (m_pfdWrite->has(fd))
			type |= SELECT_WRITE;

		if (type != SELECT_NONE)
		{
			//复制到OutEvents数组
			m_pOutEvents[nOutCount] = event;
			m_pOutEvents[nOutCount].eventType = type;
			nOutCount++;
		}
	}

	//select完毕，清空fdset
	Zero();

	return nOutCount;
}

#endif //DON_SOCKET_SELECT
