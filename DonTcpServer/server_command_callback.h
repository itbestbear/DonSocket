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

#ifndef SERVER_COMMAND_ENGINE_CALLBACK_H
#define SERVER_COMMAND_ENGINE_CALLBACK_H

#include <string>

#include "../CommandEngine/CommandEngine.h"

class CServer;
class CServerCommandEngineCallBack : public ICommandEngineCallBack
{
public:
	CServerCommandEngineCallBack(CServer* pServer);
	virtual ~CServerCommandEngineCallBack() {}

public:
	virtual bool OnInitialize();
	virtual bool OnCommand(char* szBuff);
	virtual bool OnShutdown();
public:
	virtual bool OnStartCommand(char* szName, bool result);
	virtual bool OnStopCommand(char* szName,bool isfind);

public:
	void Help();

private:
	CServer*	m_pServer;
};

#endif // SERVER_COMMAND_ENGINE_CALLBACK_H

