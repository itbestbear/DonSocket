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

#include "Executer.h"
#include "../Util/Util.h"

CExecuter::CExecuter()
{
}

CExecuter::~CExecuter()
{
	Stop();
}

int CExecuter::Create(CCommandObject* pCmdObject,int nThreadCount)
{
	if (nThreadCount <= 0)
		nThreadCount = 1;

	for (int i = 0; i < nThreadCount; i++)
	{
		m_threads.push_back(new CExecuterThread(pCmdObject,i));
	}
	
	return 1;
}

int CExecuter::Start()
{
	for (int i = 0; i < m_threads.size(); i++)
	{
		CExecuterThread* pThread = m_threads[i];
		if (!pThread->Start())
			return 0;
	}

    return 1;
}

int CExecuter::Stop()
{
	for (int i = 0; i < m_threads.size(); i++)
	{
		CExecuterThread* pThread = m_threads[i];
		if (pThread)
		{
			if (!pThread->Stop())
				return 0;
		}
	}

	return Destroy();
}

int CExecuter::WaitStop()
{
	while (true)
	{
		if (IsQuitted())
			break;

		DonSleep(1);
	}

	return Destroy();
}

int CExecuter::Destroy()
{
	for (int i = 0; i < m_threads.size(); i++)
	{
		CExecuterThread* pThread = m_threads[i];

		if (pThread)
		{
			delete pThread;
			pThread = NULL;
		}
	}
	m_threads.clear();

	return 1;
}

bool CExecuter::IsQuitted()
{
	for (int i = 0; i < m_threads.size(); i++)
	{
		CExecuterThread* pThread = m_threads[i];
		if(!pThread->IsQuitted())
			return false;
	}

	return true;
}

CExecuterThread* CExecuter::GetThread(int nThreadId)
{ 
	if (nThreadId < 0 || nThreadId >= m_threads.size())
		return NULL;

	return m_threads[nThreadId]; 
}
