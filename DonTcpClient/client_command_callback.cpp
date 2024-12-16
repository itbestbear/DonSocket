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

#include "client_command_callback.h"

#include "client.h"
#include "cmd_sendall.h"
#include "cmd_test.h"

CClientCommandEngineCallBack::CClientCommandEngineCallBack(CClient* pClient)
{
	m_pClient = pClient;
}

bool CClientCommandEngineCallBack::OnInitialize()
{
	//log_trace_always("command engine initialized");

	return true;
}

bool CClientCommandEngineCallBack::OnShutdown()
{
	//log_trace_always("command engine shutdown");

	return true;
}

bool CClientCommandEngineCallBack::OnStartCommand(char* szName, bool result)
{
	log_trace_always("start command:@c%s @r%s", szName, result ? "succeed" : "failed");

	return true;
}

bool CClientCommandEngineCallBack::OnStopCommand(char* szName, bool isfind)
{
	log_trace_always("stop command:@c%s @r%s", szName, isfind ? "succeed" : "failed");

	return true;
}

bool CClientCommandEngineCallBack::OnCommand(char* szBuff)
{
	std::vector<std::string> cmds = SplitStr(szBuff, " ");
	if (cmds.size() <= 0)
	{
		return true;
	}

	const char* szCommand = cmds[0].c_str();

	if (strcmp(szCommand, "q") == 0 || strstr(szCommand, "quit") != NULL)
	{
		return false;
	}
	else if (strcmp(szCommand, "h") == 0 || strstr(szCommand, "help") != NULL)
	{
		Help();
		return true;
	}
	else if (strcmp(szCommand, "c") == 0 || strstr(szCommand, "color") != NULL)
	{
		Console.ChangeColor();
		return true;
	}
	else if (strcmp(szCommand, "l") == 0 || strstr(szCommand, "level") != NULL)
	{
		Log.ChangeLevel(cmds);
		return true;
	}
	else if (strcmp(szCommand, "t") == 0 || strstr(szCommand, "test") != NULL)
	{
		m_pClient->GetCommandEngine()->StartCommand(new CCommand_Test("test",m_pClient, cmds, 1, 10));
		return true;
	}
	else if (strcmp(szCommand, "d") == 0 || strstr(szCommand, "sendall") != NULL)
	{
		m_pClient->GetCommandEngine()->StartCommand(new CCommand_SendAll("sendall",m_pClient, cmds, 1, 0));
		return true;
	}
	else if (strcmp(szCommand, "s") == 0 || strstr(szCommand, "stop") != NULL)
	{
		if (cmds.size() != 2)
		{
			cprintf("@Rstop command need a command name!\n@T");
			return true;
		}
		m_pClient->GetCommandEngine()->StopCommand((char*)cmds[1].c_str());
		return true;
	}
	else
	{
		cprintf("@Runknown command!\n@T");
	}

	return true;
}

void CClientCommandEngineCallBack::Help()
{
	//printf("%50s", str); //右对齐，左补空格，50是长度
	//printf("%-50s", str); //左对齐，右补空格，50是长度
	//printf("%02d", cnt); //右对齐，左补0，2是长度
	
	cprintf("@Gcommands:\n");

	cprintf("@R[q]@G%-*s@c-%s", 7, "quit", "quit the process\n");
	cprintf("@R[h]@G%-*s@c-%s", 7, "help", "help\n");
	cprintf("@R[s]@G%-*s@c-%s", 7, "stop", "stop command name\n");
	cprintf("@R[c]@G%-*s@c-%s", 7, "color", "change output color\n");
	cprintf("@R[l]@G%-*s@c-%s", 7, "level", "change log level\n");
	cprintf("@R[t]@G%-*s@c-%s", 7, "test", "start test command\n");
	cprintf("@R[d]@G%-*s@c-%s", 7, "sendall", "start sendall command\n");
}
