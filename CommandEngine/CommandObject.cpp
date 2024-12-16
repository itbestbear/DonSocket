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

#include "CommandObject.h"
#include <string.h>

CCommandObject::CCommandObject(char* szName, void* pHost, std::vector<std::string>& params, int nThreadCount, int nMillisecInterval)
{
	m_pHost = pHost;
	m_params = params;
	m_nThreadCount = nThreadCount;
	m_nMillisecInterval = nMillisecInterval;
	m_strName = szName;

	m_pExecuter = new CExecuter();
}
CCommandObject::~CCommandObject()
{
	if (m_pExecuter != NULL)
	{
		delete m_pExecuter;
		m_pExecuter = NULL;
	}
}

bool CCommandObject::Start()
{
	if (!OnCmdStart(m_params))
		return false;

	m_pExecuter->Create(this, m_nThreadCount);

	if (!m_pExecuter->Start())
		return false;

	OnCmdStarted();

	return true;
}

bool CCommandObject::Stop()
{
	if (!OnCmdStop())
		return false;

	if (IsExecOnce())
	{
		if (!m_pExecuter->WaitStop())
			return false;
	}
	else
	{
		if (!m_pExecuter->Stop())
			return false;
	}

	OnCmdStopped();

	return true;
}
