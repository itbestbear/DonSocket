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

#ifndef SERVER_H
#define SERVER_H

#include <string>

#include "../DonSocket/DonTcpServer.h"
#include "../DonSocket/Message.h"
#include "../Util/Util.h"
#include "../Util/Console.h"
#include "../CommandEngine/CommandEngine.h"

enum
{
	MAX_CLIENTS = 1000,
	MAX_THREADS = 8,
};

class CServer
{
public:
	CServer();
	virtual ~CServer();
	
public:
	bool Initialize();
	bool Run();
	bool Shutdown();
	
public:
	CDonTcpServer* GetDonTcpServer() { return m_pDonTcpServer; }
private:
	CDonTcpServer* m_pDonTcpServer;

public:
	CCommandEngine* GetCommandEngine() { return m_pCommandEngine; }
private:
	CCommandEngine* m_pCommandEngine;
};

#endif // SERVER_H

