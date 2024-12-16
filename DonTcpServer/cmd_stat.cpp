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


#include "cmd_stat.h"

#include "server.h"
#include "server_socket_callback.h"
#include "../Util/Console.h"

bool CCommand_Stat::OnCmdStart(std::vector<std::string>& params)
{
	//check params
	log_trace_fatal("command name:@R%s@T started", m_strName.c_str());

	return true; 
}

bool CCommand_Stat::OnCmdStarted()
{
	return true;
}

bool CCommand_Stat::OnCmdRun()
{ 
	CServer* pServer = (CServer*)GetHost();

	CDonTcpServer* pDonTcpServer = pServer->GetDonTcpServer();

	CDonTcpServerCallBack* pCallBack = (CDonTcpServerCallBack*)pDonTcpServer->GetCallBack();

	auto t = tTime.getElapsedSecond();
	if (t >= 1.0)
	{
		//speed
		double ftotal = pCallBack->GetTotalRecvSize();

		double fspeed = ftotal / t;

		std::string strrecv = FormatData(ftotal);
		std::string strspeed = FormatData(fspeed);

		//connections
		int nClients = pDonTcpServer->GetConnectNum();
		int nThreads = pDonTcpServer->GetWorkerNum();
		int nMaxConnectNum = pDonTcpServer->GetMaxConnectNum();

		//mempool
		int nSockPoolSize = pDonTcpServer->GetSockPoolSize();
		int nSendPoolSize = pDonTcpServer->GetSendPoolSize();
		int nRecvPoolSize = pDonTcpServer->GetRecvPoolSize();


		log_trace_fatal("@cClients:[%d/%d] @gWorkers:[%d] @bMemPool:[%d/%d/%d] @rReceived:[%s] @mSpeed<%s/s>",
			nClients, nMaxConnectNum,
			nThreads,
			nSockPoolSize, nSendPoolSize, nRecvPoolSize,
			strrecv.c_str(),
			strspeed.c_str());

		pCallBack->SetTotalRecvSize(0);

		tTime.update();
	}

	return true; 
}

bool CCommand_Stat::OnCmdStop()
{ 
	return true; 
}

bool CCommand_Stat::OnCmdStopped()
{
	log_trace_fatal("command: name:@R%s@T stopped", m_strName.c_str());
	return true;
}
