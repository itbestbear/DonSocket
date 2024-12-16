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

#ifndef UTIL_H
#define UTIL_H

#include <chrono>
#include <string>
#include <vector>

class CTimestamp
{
public:
    CTimestamp()
    {
		update();
    }
    ~CTimestamp()
    {}

    static time_t getNowInMilliSec()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    void update()
    {
		m_begin = std::chrono::high_resolution_clock::now();
    }
    /**
    *   获取当前秒
    */
    double getElapsedSecond()
    {
        return  getElapsedTimeInMicroSec() * 0.000001;
    }
    /**
    *   获取毫秒
    */
    double getElapsedTimeInMilliSec()
    {
        return this->getElapsedTimeInMicroSec() * 0.001;
    }
    /**
    *   获取微妙
    */
    long long getElapsedTimeInMicroSec()
    {
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
        
protected:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_begin;
};

std::string FormatData(double data);

std::string FormatCurTime(int time_stamp_type = 1);
void        FormatCurTime(char* buf, size_t size,int time_stamp_type = 1);

void   DonSleep(int milliseconds);

std::vector<std::string> SplitStr(std::string str, std::string pattern);

#endif // UTIL_H
