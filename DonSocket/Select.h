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

#ifndef SELECT_H
#define SELECT_H

#include "Buffer.h"

#ifdef DON_SOCKET_SELECT

#define MAX_FD_COUNT 10240 //最多支持10240个连接


#include <mutex>

class CFdSet
{
public:
	CFdSet(int nMaxFdCount=10240);
	~CFdSet();
	
	void add(SOCKET s);
	void del(SOCKET s);
	void zero();
	bool has(SOCKET s);
	fd_set* fdset();
	void copy(CFdSet& set);
	bool IsEmpty() const { return (m_nfdcount == 0); }
	int	 FdCount() const { return (m_nfdcount); }
private:
	fd_set*	m_pfdset = nullptr;
	size_t	m_nfdSize = 0;
	int		m_nfdcount;
};

enum SELECT_TYPE
{
	SELECT_NONE = 0,
	SELECT_READ = 1,
	SELECT_WRITE = 2,
};

enum SELECT_OP_TYPE
{
	SELECT_OP_ADD,
	SELECT_OP_MOD,
	SELECT_OP_DEL,
};

struct SELECT_EVENT
{
	union
	{
		void*	pVoid;
		size_t  index;
	}data;
	SOCKET		sockfd;
	int			eventType;
};

class CSelect
{
public:
	~CSelect();

public:
	bool Create(int nMaxEvents);
	void Destory();
	int	 Wait(int timeout_ms);
	bool Operates(CSocket sockfd, size_t index, int op, int type);
public:
	SELECT_EVENT* GetOutEvents() { return m_pOutEvents; }

public:
	void	SetBaseIndex(size_t index)	{ m_nBaseIndex = index; }
	size_t	GetBaseIndex() const		{ return m_nBaseIndex; }

private:
	bool Add(CSocket sockfd, size_t index, int type);
	bool Modify(CSocket sockfd, size_t index, int type);
	bool Delete(CSocket sockfd, size_t index);
	void Zero();
private:
	SELECT_EVENT*	m_pRegEvents = NULL;//注册数组
	int				m_nMaxCount  = 1;	//注册数组大小

	SELECT_EVENT*	m_pOutEvents = NULL;//发生数组，记录发生的事件

private:
	CFdSet*			m_pfdRead = NULL;
	CFdSet*			m_pfdWrite= NULL;
	SOCKET			m_maxSock = 0;
	
	std::mutex		m_mtEvent;
	size_t			m_nBaseIndex=0;
};

#endif //DON_SOCKET_SELECT

#endif // SELECT_H

