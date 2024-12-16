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

#include "client.h"
#include "client_socket_callback.h"
#include "client_command_callback.h"

CClient::CClient()
{	
	CSocket::Initialize();

	m_pDonTcpClient = new CDonTcpClient(new CDonTcpClientCallBack(this),MAX_CLIENTS, MAX_THREADS,102400,102400);

	m_pCommandEngine = new CCommandEngine(new CClientCommandEngineCallBack(this));
}

CClient::~CClient()
{
	if (m_pDonTcpClient)
	{
		delete m_pDonTcpClient;
		m_pDonTcpClient = NULL;
	}

	if (m_pCommandEngine)
	{
		delete m_pCommandEngine;
		m_pCommandEngine = NULL;
	}

	CSocket::Shutdown();
}

bool CClient::Initialize()
{
	Console.Initialize(true);

	Log.Initialize("client.log", CONFIG_LOG_LEVEL, CONFIG_LOG_MODE);

	if (!m_pDonTcpClient->Initialize("127.0.0.1", 9999, true))
	{
		log_trace_error("[init] Initialize failed");
		return false;
	}

	if (!m_pCommandEngine->Initialize())
	{
		return false;
	}

	log_trace_always("[init] please input @Rt@T and press enter to start sending message to the server,q to exit");
	log_trace_always("[init] please input @Rt@T and press enter to start sending message to the server,q to exit");
	log_trace_always("[init] please input @Rt@T and press enter to start sending message to the server,q to exit");

	return true;
}

bool CClient::Run()
{
	return m_pCommandEngine->Run();
}

bool CClient::Shutdown()
{
	m_pCommandEngine->Shutdown();

	m_pDonTcpClient->Shutdown();

	Log.Shutdown();

	Console.Shutdown();

	return true;
}
