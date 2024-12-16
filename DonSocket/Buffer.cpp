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

#include "Buffer.h"
#include "../Util/Log.h"

#ifdef DON_SOCKET_IOCP
#include "Iocp.h"
#endif

CBuffer::CBuffer()
{
	m_nSize = 0;
	m_nLast = 0;
	m_fullCount = 0;
	
	m_pBuff = NULL;

#ifdef DON_SOCKET_IOCP
	m_bIsBuilt = false;
	m_pIoData = new IOCP_DATA_BASE;
#endif
}

CBuffer::CBuffer(int nSize)
{
	m_nSize = nSize;
	m_nLast = 0;
	m_fullCount = 0;
	
	m_pBuff = new char[m_nSize];

#ifdef DON_SOCKET_IOCP
	m_bIsBuilt = false;
	m_pIoData = new IOCP_DATA_BASE;
#endif
}

void CBuffer::ReNew(int nSize)
{
	m_nSize = nSize;
	m_nLast = 0;
	m_fullCount = 0;

	//todo:if size changed
	if (m_pBuff == NULL)
	{
		m_pBuff = new char[m_nSize];
	}

#ifdef DON_SOCKET_IOCP
	m_bIsBuilt = false;
	if (m_pIoData == NULL)
	{
		m_pIoData = new IOCP_DATA_BASE;
	}
#endif
}

void CBuffer::Clear()
{
	m_nLast = 0;
	m_fullCount = 0;

#ifdef DON_SOCKET_IOCP
	m_bIsBuilt = false;
#endif
}

CBuffer::~CBuffer()
{
	if (m_pBuff)
	{
		delete[] m_pBuff;
		m_pBuff = NULL;
	}

#ifdef DON_SOCKET_IOCP
	if (m_pIoData)
	{
		delete m_pIoData;
		m_pIoData = NULL;
	}
#endif
}

bool CBuffer::PushMsg(const void* pData, int nLen)
{
	int nHeaderSize = sizeof(INNER_MSG_HEADER);

	if (m_nLast + nLen + nHeaderSize > m_nSize)
	{
		++m_fullCount;
		return false;
	}

	//写内部消息头
	INNER_MSG_HEADER header;
	header.dataLength = nLen;
	memcpy(m_pBuff + m_nLast, &header, nHeaderSize);
	m_nLast += nHeaderSize;

	//写实际数据
	memcpy(m_pBuff + m_nLast, pData, nLen);
	m_nLast += nLen;

	return true;
}

bool CBuffer::PushMsg2(const void* pData1, int nLen1, const void* pData2, int nLen2)
{
	int nHeaderSize = sizeof(INNER_MSG_HEADER);

	if (m_nLast + nLen1 + nLen2 + nHeaderSize > m_nSize)
	{
		++m_fullCount;
		return false;
	}

	//写内部消息头
	INNER_MSG_HEADER header;
	header.dataLength = nLen1 + nLen2;
	memcpy(m_pBuff + m_nLast, &header, nHeaderSize);
	m_nLast += nHeaderSize;

	//写实际数据1
	memcpy(m_pBuff + m_nLast, pData1, nLen1);
	m_nLast += nLen1;

	//写实际数据2
	memcpy(m_pBuff + m_nLast, pData2, nLen2);
	m_nLast += nLen2;
		
	return true;
}

bool CBuffer::WriteSocket(CSocket& sock)
{
	if (!sock.IsValid())
		return false;

	//缓冲区有数据
	if (m_nLast > 0)
	{
		//发送数据
		int nSentLen = sock.Send(m_pBuff, m_nLast);
		if (nSentLen <= 0)
		{
			return false;//return SOCKET_ERROR;
		}

		if (nSentLen == m_nLast)//发完
		{
			m_nLast = 0;
		}
		else
		{
			m_nLast -= nSentLen;
			memcpy(m_pBuff, m_pBuff + nSentLen, m_nLast);
		}
		m_fullCount = 0;
	}

	return true;
}

bool CBuffer::ReadSocket(CSocket sock)
{
	if (m_nSize - m_nLast > 0)
	{
		//接收客户端数据
		char* szRecv = m_pBuff + m_nLast;
		int nLen = sock.Recv(szRecv, m_nSize - m_nLast);
		if (nLen <= 0)
		{
			return false;//return SOCKET_ERROR;
		}
		//消息缓冲区的数据尾部位置后移
		m_nLast += nLen;
		
		return true;//接收到数据,长度nLen
	}

	log_trace_error("[ReadSocket] recv buffer is full");

	return true;//接收缓冲区满
}

std::string CBuffer::Dump()
{
	char buf[128];
	snprintf(buf, 128, "m_nLast:%d m_nSize:%d m_fullcount:%d", m_nLast, m_nSize, m_fullCount);
	return buf;
}

std::string CBuffer::MiniDump()
{
	char buf[128];
	snprintf(buf, 128, "[L:%d S:%d F:%d]", m_nLast, m_nSize, m_fullCount);
	return buf;
}

int CBuffer::PopMsg(char* buff, size_t size)
{
	unsigned short HEADSIZE = sizeof(INNER_MSG_HEADER);

	INNER_MSG_HEADER* head = (INNER_MSG_HEADER*)m_pBuff;
	
	if (m_nLast == 0) //缓冲区没有数据了
		return BUFFER_EMPTY;

	if ( (m_nLast <= HEADSIZE) || ( m_nLast < HEADSIZE + head->dataLength)) //不够一包
		return BUFFER_WAITING;

	//完整数据
	char* msg = m_pBuff + HEADSIZE;			//实际数据部分
	unsigned short len = head->dataLength;	//实际数据长度

	int nRet = len;
	if (len > size) //输出缓存长度不够,从接收缓存中删除消息，链路仍然继续处理
	{
		nRet = BUFFER_ERROR; 
	}
	else
	{
		memcpy(buff, msg, len);//拷贝到输出缓存
	}

	//接收缓存剩余数据前移
	int n = m_nLast - (len+HEADSIZE);
	if (n > 0)
	{
		memcpy(m_pBuff, m_pBuff + (len + HEADSIZE), n);
	}
	m_nLast = n;
	if (m_fullCount > 0)
		--m_fullCount;
	
	return nRet;
}

#ifdef DON_SOCKET_IOCP
IOCP_DATA_BASE* CBuffer::BuildRecvIoData(CSocket sockfd)
{
	if (m_bIsBuilt)
		return NULL;

	m_bIsBuilt = true;

	int nLen = m_nSize - m_nLast;
	if (nLen > 0)
	{
		m_pIoData->wsabuff.buf = m_pBuff + m_nLast;
		m_pIoData->wsabuff.len = nLen;
		m_pIoData->sockfd = sockfd.GetSocket();
		return m_pIoData;
	}
	return nullptr;
}

bool CBuffer::ReadIocp(int nRecv)
{
	if (!m_bIsBuilt)
	{

	}

	m_bIsBuilt = false;

	if (nRecv > 0 && m_nSize - m_nLast >= nRecv)
	{
		m_nLast += nRecv;
		return true;
	}

	return false;
}

IOCP_DATA_BASE* CBuffer::BuildSendIoData(CSocket sockfd)
{
	if (m_bIsBuilt)
		return NULL;
	m_bIsBuilt = true;

	if (m_nLast > 0)
	{
		m_pIoData->wsabuff.buf = m_pBuff;
		m_pIoData->wsabuff.len = m_nLast;
		m_pIoData->sockfd = sockfd.GetSocket();
		return m_pIoData;
	}
	return nullptr;
}

bool CBuffer::WriteIocp(int nSend)
{
	if (!m_bIsBuilt)
	{

	}

	m_bIsBuilt = false;

	if (m_nLast < nSend)
	{
		return false;
	}
	if (m_nLast == nSend)
	{
		//发完
		m_nLast = 0;
	}
	else
	{
		//发了一部分
		m_nLast -= nSend;
		memcpy(m_pBuff, m_pBuff + nSend, m_nLast);
	}
	m_fullCount = 0;
	return true;
}

#endif // DON_SOCKET_IOCP
