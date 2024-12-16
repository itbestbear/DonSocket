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

#include "cmd_test.h"

#include "client.h"

bool CCommand_Test::OnCmdStart(std::vector<std::string>& params)
{
	//check params
	log_trace_fatal("command name:@R%s@T started", m_strName.c_str());

	//std::vector<std::string>& params = GetParams();
	std::string strCmd = params[0];

	return true; 
}

bool CCommand_Test::OnCmdStarted()
{
	return true;
}

bool CCommand_Test::OnCmdRun()
{ 
	CClient* pClient = (CClient*)GetHost();
	CDonTcpClient* pDonTcpClient = pClient->GetDonTcpClient();

	int nClients = pDonTcpClient->GetConnectNum();
	int nMaxClients = pDonTcpClient->GetMaxConnectNum();
	int nWorkers = pDonTcpClient->GetWorkerNum();

	//mempool
	int nSockPoolSize = pDonTcpClient->GetSockPoolSize();
	int nSendPoolSize = pDonTcpClient->GetSendPoolSize();
	int nRecvPoolSize = pDonTcpClient->GetRecvPoolSize();


	MSG_ECHO msg;
	msg.nType = MT_ECHO;
	const char* data = "hello server,this is a test message from client";
	int len = strlen(data);
	strcpy(msg.echo, data);
	FormatCurTime(msg.time, sizeof(msg.time));


	int nCycleTimes = 1;
	for (int i = 0; i < nCycleTimes; i++)
	{
		bool bResult = pDonTcpClient->ClientSendAll(&msg, sizeof(msg));
		log_trace_debug("[send] type:%d time:%s msg:%s len:%d result:%s", msg.nType, msg.time, msg.echo, len, bResult ? "succeed" : "failed");
	}

	static double total = 0;
	double dPayload = sizeof(msg) * nCycleTimes * nClients;
	total += dPayload;

	double tUsed = tTime.getElapsedSecond();
	if (tUsed >= 1)
	{
		double fspeed = total / tUsed;

		std::string strsend = FormatData(total);
		std::string strspeed = FormatData(fspeed);

		log_trace_fatal("@cConnects:[%d/%d] @gWorkers:[%d] @bMemPool:[%d/%d/%d] @rSent:[%s] @mSpeed<%s/s>",
			nClients, nMaxClients,
			nWorkers,
			nSockPoolSize, nSendPoolSize, nRecvPoolSize,
			strsend.c_str(),
			strspeed.c_str()
		);

		total = 0;
		tTime.update();
	}

	return true; 
}

bool CCommand_Test::OnCmdStop()
{ 
	return true; 
}

bool CCommand_Test::OnCmdStopped()
{
	log_trace_fatal("command name:@R%s@T stopped", m_strName.c_str());
	return true;
}
