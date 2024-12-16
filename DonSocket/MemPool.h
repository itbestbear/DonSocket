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

#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <mutex>

template<typename TYPE>
class CMemPool
{
private:
	struct POOL_DATA
	{
		TYPE		data;//这里不能用指针，否则data指针不能转为POOL_DATA指针
		POOL_DATA* pNext;
	};

public:
	CMemPool()
	{
		m_pFreeList = NULL;
	}

	~CMemPool()
	{
		Clear();
	}

	TYPE* New()
	{
		if (m_pFreeList)
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			if (m_pFreeList)
			{
				POOL_DATA* p = m_pFreeList;

				m_pFreeList = p->pNext;

				return &p->data;
			}
		}

		POOL_DATA* p = new POOL_DATA;

		p->pNext = NULL;

		return &p->data;
	}

	void Delete(TYPE* pData)
	{
		std::lock_guard<std::mutex> lock(m_Lock);

		POOL_DATA* p = (POOL_DATA*)pData;//TYPE* data指针转为POOL_DATA指针

		p->pNext = m_pFreeList;

		m_pFreeList = p;
	}

	size_t Count()
	{
		std::lock_guard<std::mutex> lock(m_Lock);

		size_t count = 0;

		POOL_DATA* p = m_pFreeList;

		while (p)
		{
			++count;
			p = p->pNext;
		}

		return count;
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(m_Lock);

		POOL_DATA* p;
		POOL_DATA* t;

		p = m_pFreeList;

		while (p)
		{
			t = p;
			p = p->pNext;
			delete t;
		}

		m_pFreeList = NULL;
	}

private:
	POOL_DATA* m_pFreeList;
	std::mutex	m_Lock;
};

#endif // MEMPOOL_H
