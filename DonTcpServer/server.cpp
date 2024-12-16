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

#include "server.h"
#include "server_socket_callback.h"
#include "server_command_callback.h"

CServer::CServer()
{	
	CSocket::Initialize();

	m_pDonTcpServer = new CDonTcpServer(new CDonTcpServerCallBack(this),MAX_CLIENTS,MAX_THREADS,102400,102400);

	m_pCommandEngine = new CCommandEngine(new CServerCommandEngineCallBack(this));
}

CServer::~CServer()
{
	if (m_pDonTcpServer)
	{
		delete m_pDonTcpServer;
		m_pDonTcpServer = NULL;
	}
	
	if (m_pCommandEngine)
	{
		delete m_pCommandEngine;
		m_pCommandEngine = NULL;
	}
	
	CSocket::Shutdown();
}

bool CServer::Initialize()
{
	Console.Initialize(true);

	Log.Initialize("server.log", CONFIG_LOG_LEVEL, CONFIG_LOG_MODE);

	if (!m_pDonTcpServer->Initialize("127.0.0.1", 9999))
	{
		return false;
	}

	if (!m_pCommandEngine->Initialize())
	{
		return false;
	}

	log_trace_always("[init] please input @Ra@T and press enter to run the stastics thread of the server,q to exit");
	log_trace_always("[init] please input @Ra@T and press enter to run the stastics thread of the server,q to exit");
	log_trace_always("[init] please input @Ra@T and press enter to run the stastics thread of the server,q to exit");
	
	return true;
}

bool CServer::Run()
{
	return m_pCommandEngine->Run();
}

bool CServer::Shutdown()
{
	m_pCommandEngine->Shutdown();

	m_pDonTcpServer->Shutdown();

	Log.Shutdown();

	Console.Shutdown();

	return true;
}
