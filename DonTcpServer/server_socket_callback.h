
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

#ifndef SERVER_CALLBACK_H
#define SERVER_CALLBACK_H

#include <string>

#include "../DonSocket/DonTcpServer.h"
#include "../DonSocket/Message.h"

class CServer;
class CDonTcpServerCallBack : public IDonTcpServerCallBack
{
public:
	CDonTcpServerCallBack(CServer* pServer);
	virtual ~CDonTcpServerCallBack() {}

	virtual int OnAccept(size_t index, int serial, const char* addr, int port);
	virtual int OnClose(size_t index, int serial, const char* addr, int port);
	virtual int OnReceive(size_t index, int serial, const void* pdata, size_t len);

private:
	CDonTcpServerCallBack();

public:
	double GetTotalRecvSize() {return m_nTotalRecvSize;}
	void SetTotalRecvSize(double n) { m_nTotalRecvSize = n; }
private:
	std::atomic<double> m_nTotalRecvSize;// = 0.0; //接收数据大小

private:
	CServer*	m_pServer;
};

#endif // SERVER_CALLBACK_H

