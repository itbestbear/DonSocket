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

#ifndef CLIENT_CALLBACK_H
#define CLIENT_CALLBACK_H

#include <string>

#include "../DonSocket/DonTcpClient.h"
#include "../DonSocket/Message.h"

class CClient;
class CDonTcpClientCallBack : public IDonTcpClientCallBack
{
public:
	CDonTcpClientCallBack(CClient* pClient);
	virtual ~CDonTcpClientCallBack() {}

	virtual int OnConnect(size_t index, const char* addr, int port);
	virtual int OnClose(size_t index, const char* addr, int port);
	virtual int OnReceive(size_t index, const void* pdata, size_t len);

private:
	CDonTcpClientCallBack();

private:
	CClient*	m_pClient;
};

#endif // CLIENT_CALLBACK_H

