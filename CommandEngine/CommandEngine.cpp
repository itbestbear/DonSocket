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


#include "CommandEngine.h"
#include <string.h>
#include "../Util/Log.h"

CCommandEngine::CCommandEngine(ICommandEngineCallBack* pCmdEngineCallBack)
{
	m_pCmdEngineCallBack = pCmdEngineCallBack;
}

CCommandEngine::~CCommandEngine()
{
	if (m_pCmdEngineCallBack)
	{
		delete m_pCmdEngineCallBack;
		m_pCmdEngineCallBack = NULL;
	}
}

bool CCommandEngine::Initialize()
{
	if (!m_pCmdEngineCallBack->OnInitialize())
		return false;

	log_trace_always("[init] command engine initialized");

	return true;
}

bool CCommandEngine::Run()
{
	char buffer[1024] = { 0 };
	memset(buffer, 0, sizeof(buffer));
	char* p = NULL;
	bool result = false;

	while ((p = fgets(buffer, sizeof(buffer), stdin)) != NULL)
	{
		buffer[strcspn(buffer, "\n")] = ' ';//把最后的回车替换为空格

		result = m_pCmdEngineCallBack->OnCommand(buffer);
		if (!result)
			break;
	}

	return result;
}

bool CCommandEngine::Shutdown()
{
	for (int i = 0; i < m_cmdobjects.size(); i++)
	{
		CCommandObject* pCmdObject = m_cmdobjects[i];
		if (pCmdObject == NULL)
			continue;

		pCmdObject->Stop();
		delete pCmdObject;
	}
	m_cmdobjects.clear();

	m_pCmdEngineCallBack->OnShutdown();

	log_trace_always("[shut] command engine shutdown");

	return true;
}

bool CCommandEngine::StartCommand(CCommandObject* pCmdObject)
{
	if (!pCmdObject)
		return false;

	if (FindCommand((char*)pCmdObject->GetName().c_str())
		|| (!pCmdObject->Start()))
	{
		m_pCmdEngineCallBack->OnStartCommand((char*)pCmdObject->GetName().c_str(), false);

		delete pCmdObject;
		pCmdObject = NULL;
		return false;
	}

	m_pCmdEngineCallBack->OnStartCommand((char*)pCmdObject->GetName().c_str(),true);

	if (!pCmdObject->IsExecOnce())
	{
		m_cmdobjects.push_back(pCmdObject);
	}
	else
	{
		pCmdObject->Stop();
		
		m_pCmdEngineCallBack->OnStopCommand((char*)pCmdObject->GetName().c_str(), true);

		delete pCmdObject;
		pCmdObject = NULL;
	}

	return true;
}

bool CCommandEngine::StopCommand(char* szName)
{
	bool find = false;
	for (std::vector<CCommandObject*>::iterator iter = m_cmdobjects.begin(); iter != m_cmdobjects.end();)
	{
		CCommandObject* pObject = *iter;
		if (pObject && pObject->GetName() == szName)
		{
			pObject->Stop();
			delete pObject;
			pObject = NULL;

			iter = m_cmdobjects.erase(iter);
			find = true;
			break;
		}
		else
		{
			++iter;
		}
	}

	m_pCmdEngineCallBack->OnStopCommand(szName, find);

	return true;
}

bool CCommandEngine::FindCommand(char* szName)
{
	for (int i = 0; i < m_cmdobjects.size(); i++)
	{
		CCommandObject* pObject = m_cmdobjects[i];
		if (pObject == NULL)
			continue;

		if (pObject && pObject->GetName() == szName)
			return true;
	}

	return false;
}


