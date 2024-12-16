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

#ifndef COMMAND_OBJECT_H
#define COMMAND_OBJECT_H

#include "Executer.h"

#include <string>
#include <vector>

class CCommandObject
{
public:
	//nMillisecInterval = 0 means only run one time;
	//nMillisecInterval > 0 means runs every nMillisecInterval milliseconds.
	CCommandObject(char* szName, void* pHost, std::vector<std::string>& params, int nThreadCount, int nMillisecInterval);
	virtual ~CCommandObject();

public:
	bool Start();
	bool Stop();

public:
	virtual bool OnCmdStart(std::vector<std::string>& params) = 0;
	virtual bool OnCmdStarted() = 0;
	virtual bool OnCmdRun() = 0;
	virtual bool OnCmdStop() = 0;
	virtual bool OnCmdStopped() = 0;

public:
	std::vector<std::string>& GetParams() { return m_params; }
	void* GetHost() { return m_pHost; }
	int GetMillisecInterval() { return m_nMillisecInterval; }
	bool IsExecOnce() { return m_nMillisecInterval <= 0; }
	std::string& GetName() { return m_strName; }

private:
	std::vector<std::string> m_params;
	int			m_nThreadCount;
	int			m_nMillisecInterval;
	void*		m_pHost;
protected:
	std::string	m_strName;

private:
	CExecuter* m_pExecuter;
};

#endif // COMMAND_OBJECT_H

