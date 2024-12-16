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


CDonTcpServerCallBack::CDonTcpServerCallBack(CServer* pServer)
{
	m_pServer = pServer;
	m_nTotalRecvSize = 0.0;
}

int CDonTcpServerCallBack::OnAccept(size_t index, int serial, const char* addr, int port)
{
	log_trace_debug("[onaccept] [conns:%d] index:%d serial:%d addr:%s port:%d Accepted", m_pServer->GetDonTcpServer()->GetConnectNum(), index, serial, addr, port);

	return 1;
}

int CDonTcpServerCallBack::OnClose(size_t index, int serial, const char* addr, int port)
{
	log_trace_debug("[onclose] [conns:%d] index:%d serial:%d addr:%s port:%d Closed", m_pServer->GetDonTcpServer()->GetConnectNum(), index, serial, addr, port);

	return 1;
}

int CDonTcpServerCallBack::OnReceive(size_t index, int serial, const void* pdata, size_t len)
{
	MSG_BASE* pMsg = (MSG_BASE*)pdata;
	switch (pMsg->nType)
	{
	case MT_ECHO:
	{
		m_nTotalRecvSize = m_nTotalRecvSize + len;

		log_trace_debug("[onrecv] [conns:%d] index:%d type:%d total:%f msg:%s,time:%s,dump:%s",
			m_pServer->GetDonTcpServer()->GetConnectNum(), index, pMsg->nType, m_nTotalRecvSize.load(), ((MSG_ECHO*)pMsg)->echo, ((MSG_ECHO*)pMsg)->time, m_pServer->GetDonTcpServer()->Dump(index).c_str());
	}
	break;
	default:
		break;
	}

	return 1;
}
