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

#include "client_socket_callback.h"
#include "client.h"

CDonTcpClientCallBack::CDonTcpClientCallBack(CClient* pClient)
{
	m_pClient = pClient;
}

int CDonTcpClientCallBack::OnConnect(size_t index, const char* addr, int port)
{
	log_trace_debug("[onconnect]index:%d addr:%s port:%d Connected", index, addr, port);

	return 1;
}

int CDonTcpClientCallBack::OnClose(size_t index, const char* addr, int port)
{
	log_trace_debug("[onclose]index:%d addr:%s port:%d Closed", index, addr, port);

	return 1;
}

int CDonTcpClientCallBack::OnReceive(size_t index, const void* pdata, size_t len)
{
	MSG_BASE* pMsg = (MSG_BASE*)pdata;
	switch (pMsg->nType)
	{
	case MT_ECHO:
	{
		log_trace_debug("[onrecv]index:%d type:%d msg:%s", index, pMsg->nType, ((MSG_ECHO*)pMsg)->echo);
	}
	break;
	default:
		break;
	}

	return 1;
}