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

#ifndef BUFFER_H
#define BUFFER_H

#include "Socket.h"

#define MAGIC_CODE 12321

struct INNER_MSG_HEADER
{
	INNER_MSG_HEADER()
	{
		dataLength = sizeof(INNER_MSG_HEADER);
		crc = MAGIC_CODE;
	}
	unsigned short dataLength;
	unsigned short crc;
};

#ifdef DON_SOCKET_IOCP
struct IOCP_DATA_BASE;
#endif

enum BufferState
{
	BUFFER_WAITING	=  0,//
	BUFFER_ERROR	= -1,
	BUFFER_EMPTY	= -2,
};

class CBuffer
{
public:
	CBuffer();
	CBuffer(int nSize);
	~CBuffer();

public:
	void ReNew(int nSize);
	void Clear();
public:
	char*GetBuff() const { return m_pBuff; }
	int	 GetSize() const { return m_nSize; }
	bool NeedWrite() { return m_nLast > 0; }
	bool ReadOver() { return m_nLast <= 0; }

public:
	bool PushMsg(const void* pData, int nLen);
	bool PushMsg2(const void* pData1, int nLen1, const void* pData2, int nLen2);
	int	 PopMsg(char* buff,size_t size);

public:
	bool ReadSocket(CSocket sock);
	bool WriteSocket(CSocket& sock);
	bool ReadIocp(int nRecv);
	bool WriteIocp(int nSend);

public:
	std::string Dump();
	std::string MiniDump();

private:
	char* m_pBuff;		//缓冲区

	int m_nLast;		//缓冲区的数据尾部位置，已有数据长度
	int m_nSize;		//缓冲区总的空间大小，字节长度
	int m_fullCount;	//缓冲区写满次数计数

#ifdef DON_SOCKET_IOCP
public:
	IOCP_DATA_BASE* BuildRecvIoData(CSocket sockfd);
	IOCP_DATA_BASE* BuildSendIoData(CSocket sockfd);
private:
	IOCP_DATA_BASE* m_pIoData;
	bool m_bIsBuilt;	//IOCP下用于投递事件
#endif // DON_SOCKET_IOCP
};

#endif // BUFFER_H
