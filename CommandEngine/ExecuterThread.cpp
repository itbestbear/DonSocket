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

#include "ExecuterThread.h"
#include "CommandEngine.h"
#include "../Util/Util.h"

CExecuterThread::CExecuterThread(CCommandObject* pCmdObject, int nThreadId)
{
	m_quit_flag	= EXECUTER_UNOPEN;

	m_pCmdObject = pCmdObject;

	m_nThreadId = nThreadId;

	m_pThread = NULL;
}

CExecuterThread::~CExecuterThread()
{

}

void executerThreadFunc(void* arg)
{
	CExecuterThread* pThread = (CExecuterThread*)arg;
	
    if(pThread)
    {
		pThread->Run();
    }
}

int CExecuterThread::Start()
{
	if (m_quit_flag == EXECUTER_RUNNING)
	{
		return 0;
	}
		
    m_quit_flag = EXECUTER_RUNNING;
    m_pThread = new std::thread(executerThreadFunc,this);

    return 1;
}

int CExecuterThread::Stop()
{
    if(m_quit_flag != EXECUTER_RUNNING)
        return 1;
	
	m_quit_flag = EXECUTER_QUIT;

    m_pThread->join();

    if(m_pThread!=NULL)
    {
        delete m_pThread;
        m_pThread=NULL;
    }

	return 1;
}

int CExecuterThread::Run()
{
	while (m_quit_flag != EXECUTER_QUIT)
	{
		m_pCmdObject->OnCmdRun();

		if (m_pCmdObject->IsExecOnce())
		{
			break;
		}
		else
		{
			int nMillisecInterval = m_pCmdObject->GetMillisecInterval();
			DonSleep(nMillisecInterval);
		}
	}

	m_quit_flag = EXECUTER_QUITED;

	return 1;
}

