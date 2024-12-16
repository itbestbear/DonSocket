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

#ifndef EXECUTER_THREAD_H
#define EXECUTER_THREAD_H

#include <atomic>
#include <thread>

enum
{
	EXECUTER_UNOPEN,  
	EXECUTER_RUNNING, 
	EXECUTER_QUIT,	
	EXECUTER_QUITED,	
};

class CCommandObject;
class CExecuterThread
{
public:
	CExecuterThread(CCommandObject* pCmdObject, int nThreadId);
	virtual ~CExecuterThread();

public:
	int Start();
	int Run();
	int Stop();
	
public:
	bool IsQuitted() { return m_quit_flag == EXECUTER_QUITED; }
	
		
public:
    std::thread*	m_pThread;
	int				m_quit_flag;

private:
	int m_nThreadId;
	CCommandObject* m_pCmdObject;
};

#endif // EXECUTER_THREAD_H
