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

#include "Util.h"
#include <thread>

std::string FormatData(double data)
{
	char buf[128];

	if (data >= 1024.0 * 1024.0)
	{
		sprintf(buf, "%.2fmb", data / (1024.0 * 1024.0));
	}
	else if (data >= 1024.0)
	{
		sprintf(buf, "%.2fkb", data / (1024.0));
	}
	else
	{
		sprintf(buf, "%.2fb", data);
	}

	return buf;
}

void   DonSleep(int milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

std::vector<std::string> SplitStr(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//扩展字符串以方便操作
	size_t size = str.size();
	for (size_t i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);

			if (!s.empty())
				result.push_back(s);

			i = pos + pattern.size() - 1;
		}
	}
	return result;
}


std::string FormatCurTime(int time_stamp_type)
{
	char buffer[128];
	FormatCurTime(buffer, sizeof(buffer), time_stamp_type);
	return buffer;
}

void FormatCurTime(char* buf, size_t size,int time_stamp_type)
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm* now_tm = std::localtime(&now_time_t);
	
	char buffer[128];
	strftime(buffer, sizeof(buffer), "%F %T", now_tm);

	std::chrono::milliseconds ms;
	std::chrono::microseconds cs;
	std::chrono::nanoseconds ns;

	switch (time_stamp_type)
	{
	case 0:
		snprintf(buf, size, "[%s]", buffer);
		break;
	case 1:
		ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
		snprintf(buf, size, "[%s %.3lld]",buffer,ms.count());
		break;
	case 2:
		ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
		cs = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
		snprintf(buf, size, "[%s %.3lld.%.3lld]", buffer, ms.count(), cs.count() % 1000);
		break;
	case 3:
		ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
		cs = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
		ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()) % 1000000000;
		snprintf(buf, size, "[%s %.3lld.%.3lld.%.3lld]", buffer, ms.count(), cs.count() % 1000, ns.count() % 1000);
		break;
	default:
		snprintf(buf, size, "[%s]", buffer);
		break;
	}
}
