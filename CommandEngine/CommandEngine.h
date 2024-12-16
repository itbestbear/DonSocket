
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


#ifndef COMMAND_ENGINE_H
#define COMMAND_ENGINE_H

#include "CommandObject.h"

#include <string>
#include <vector>

class ICommandEngineCallBack
{
public:
	virtual ~ICommandEngineCallBack() {}

public:
	virtual bool OnInitialize() = 0;
	virtual bool OnCommand(char* szBuff) = 0;
	virtual bool OnShutdown() = 0;
public:
	virtual bool OnStartCommand(char* szName,bool result) = 0;
	virtual bool OnStopCommand(char* szName, bool isfind) = 0;
};

class CCommandEngine
{
public:
	CCommandEngine(ICommandEngineCallBack* pCmdEngineCallBack);
	virtual ~CCommandEngine();
	
public:
	bool Initialize();
	bool Run();
	bool Shutdown();

public:
	bool StartCommand(CCommandObject* pCmdObject);
	bool StopCommand(char* szName);
	bool FindCommand(char* szName);

public:
	ICommandEngineCallBack* GetCallBack() { return m_pCmdEngineCallBack; }

private:
	std::vector<CCommandObject*>	m_cmdobjects;

private:
	ICommandEngineCallBack*			m_pCmdEngineCallBack;

};

#endif // COMMAND_ENGINE_H

