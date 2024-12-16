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

#ifndef EXECUTER_H
#define EXECUTER_H

#include "ExecuterThread.h"

#include <vector>
#include <string>

class CCommandObject;
class CExecuter
{
public:
	CExecuter();
	virtual ~CExecuter();

public:
	int Create(CCommandObject* pCmdObject,int nThreadCount);
	int Start();
	int Stop();
	int WaitStop();
	int Destroy();
private:
	bool IsQuitted();

public:
	CExecuterThread* GetThread(int nThreadId);
private:
    std::vector<CExecuterThread*>   m_threads;
};


#endif // EXECUTER_H
