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

#include "Console.h"
#include <stdio.h>
#include <stdarg.h>
#include "Util.h"

#ifdef _WIN32
WORD wOldColorAttrs = 0;
void GetDefault()
{
    if (wOldColorAttrs != 0)
        return;

    HANDLE handle;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(handle, &csbiInfo);
    wOldColorAttrs = csbiInfo.wAttributes;
}

void SetColor(unsigned short forecolor, unsigned short backgroudcolor)
{
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);

    if (forecolor == 0 && backgroudcolor == 0)
    {
        SetConsoleTextAttribute(hCon, wOldColorAttrs);
        return;
    }

    SetConsoleTextAttribute(hCon, forecolor | backgroudcolor);
}
#endif

CConsole::CConsole() 
{
	m_colorfull = false;
	m_color_last = 'D';
	m_color_current = 'D';
}

bool CConsole::Initialize(bool colorfull) 
{
    m_colorfull = colorfull;

#ifdef _WIN32
    GetDefault();
#endif

    return true;
}
 
void CConsole::Shutdown()
{
	cprintf("@Wpress ENTER to exit!");

    int ret = getchar();
}

void CConsole::ChangeColor()
{
	cprintf("@Tcolor changed!\n");

    m_colorfull = !m_colorfull;
	if (!m_colorfull)
	{
		DefaultColor();
	}
}

void CConsole::PrintColor(char color)
{
	if (!m_colorfull)
	{
		return;
	}

	if (color == 'T')
	{
		RESET;
		return;
	}

	m_color_last = m_color_current;

	switch (color)
	{
	case 'r':RED;		break;
	case 'R':BOLDRED;	break;
	case 'g':GREEN;		break;
	case 'G':BOLDGREEN;	break;
	case 'b':BLUE;		break; 
	case 'B':BOLDBLUE;	break; 
	case 'y':YELLOW;	break;
	case 'Y':BOLDYELLOW;break;
	case 'w':WHITE;		break;
	case 'W':BOLDWHITE;	break;
	case 'k':BLACK;		break;
	case 'K':BOLDBLACK;	break;
	case 'c':CYAN;		break;
	case 'C':BOLDCYAN;	break;
	case 'm':MAGENTA;	break;
	case 'M':BOLDMAGENTA;break;
	case 'D':DEFAULT;	break;
	default:
		break;
	}

	m_color_current = color;
}

void CConsole::ResetColor()
{
	if (!m_colorfull)
	{
		return;
	}

	switch (m_color_last)
	{
	case 'r':RED;		break;
	case 'R':BOLDRED;	break;
	case 'g':GREEN;		break;
	case 'G':BOLDGREEN;	break;
	case 'b':BLUE;		break;
	case 'B':BOLDBLUE;	break;
	case 'y':YELLOW;	break;
	case 'Y':BOLDYELLOW; break;
	case 'w':WHITE;		break;
	case 'W':BOLDWHITE;	break;
	case 'k':BLACK;		break;
	case 'K':BOLDBLACK;	break;
	case 'c':CYAN;		break;
	case 'C':BOLDCYAN;	break;
	case 'm':MAGENTA;	break;
	case 'M':BOLDMAGENTA;break;
	//case 'T':RESET;		break;
	case 'D':DEFAULT;	break;
	default:
		break;
	}

	m_color_current = m_color_last;
}

void CConsole::DefaultColor()
{
	DEFAULT;
}

std::string cprintf(const char* fmt, ...)
{
	//first format
	char buf[512];

	va_list va;
	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	//then split
	std::vector<std::string> vec = SplitStr(buf, "@");

	std::string strData;
	for (int i = 0; i < vec.size(); i++)
	{
		std::string& item = vec[i];

		char color = item[0];
		Console.PrintColor(color);

		item = item.substr(1, item.size());
		printf(item.c_str());

		strData += item;
	}
	return strData;
}



