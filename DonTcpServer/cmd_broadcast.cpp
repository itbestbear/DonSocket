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

#include "cmd_broadcast.h"

#include "server.h"
#include "../Util/Console.h"

bool CCommand_Broadcast::OnCmdStart(std::vector<std::string>& params)
{
	log_trace_fatal("command name:@R%s@T started", m_strName.c_str());

	if (params.size() != 2)
	{
		log_trace_error("CCommand_Broadcast::OnCmdStart,wrong params");

		return false;
	}

	return true;
}

bool CCommand_Broadcast::OnCmdStarted()
{
	return true;
}

bool CCommand_Broadcast::OnCmdRun()
{
	CServer* pServer = (CServer*)GetHost();

	std::vector<std::string>& params = GetParams();


	MSG_ECHO msg;
	msg.nType = MT_ECHO;

	int nSize = sizeof(msg.echo);
	int nLen = strlen(params[1].c_str());
	if (nLen > nSize)
	{
		log_trace_error("[send][error] type:%d msg:%s too long,cache:%d msg:%d", msg.nType, params[1].c_str(), nSize, nLen);
	}
	else
	{
		strcpy(msg.echo, params[1].c_str());
		FormatCurTime(msg.time, sizeof(msg.time));
		bool bResult = pServer->GetDonTcpServer()->Broadcast(&msg, sizeof(msg));

		log_trace_debug("[send] type:%d msg:%s len:%d result:%s", msg.nType, msg.echo, nLen, bResult ? "succeed" : "failed");
	}

	return true;
}

bool CCommand_Broadcast::OnCmdStop()
{
	return true;
}

bool CCommand_Broadcast::OnCmdStopped()
{
	log_trace_fatal("command name:@R%s@T stopped", m_strName.c_str());
	return true;
}
