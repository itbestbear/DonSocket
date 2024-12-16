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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
void GetDefault();
void SetColor(unsigned short forecolor = 4, unsigned short backgroudcolor = 0);
#define BOLDRED		 SetColor(FOREGROUND_RED,0);
#define BOLDBLUE	 SetColor(FOREGROUND_BLUE,0);
#define BOLDGREEN	 SetColor(FOREGROUND_GREEN,0);
#define BOLDYELLOW	 SetColor(FOREGROUND_RED | FOREGROUND_GREEN,0);
#define BOLDWHITE	 SetColor(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN,0);
#define BOLDMAGENTA	 SetColor(FOREGROUND_RED | FOREGROUND_BLUE,0);
#define BOLDCYAN	 SetColor(FOREGROUND_GREEN | FOREGROUND_BLUE,0);
#define RED			 SetColor(FOREGROUND_RED|FOREGROUND_INTENSITY,0);
#define BLUE		 SetColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY,0);
#define GREEN		 SetColor(FOREGROUND_GREEN|FOREGROUND_INTENSITY,0);
#define YELLOW		 SetColor(FOREGROUND_RED | FOREGROUND_GREEN|FOREGROUND_INTENSITY,0);
#define WHITE		 SetColor(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY,0);
#define MAGENTA		 SetColor(FOREGROUND_RED | FOREGROUND_BLUE|FOREGROUND_INTENSITY,0);
#define CYAN		 SetColor(FOREGROUND_GREEN | FOREGROUND_BLUE|FOREGROUND_INTENSITY,0);
#define BLACK		 SetColor(0,0);
#define BOLDBLACK	 SetColor(0,0);
#define RESET		 ResetColor();
#define DEFAULT		 SetColor(0,0);
#else
//the following are UBUNTU/LINUX ONLY terminal color codes.
#define DEFAULT		 printf("\033[0m");
#define RESET		 ResetColor();
#define BLACK		 printf("\033[30m");      /* Black */
#define RED			 printf("\033[31m");      /* Red */
#define GREEN		 printf("\033[32m");      /* Green */
#define YELLOW		 printf("\033[33m");      /* Yellow */
#define BLUE		 printf("\033[34m");      /* Blue */
#define MAGENTA		 printf("\033[35m");      /* Magenta */
#define CYAN		 printf("\033[36m");      /* Cyan */
#define WHITE		 printf("\033[37m");      /* White */
#define BOLDBLACK    printf("\033[1m\033[30m");      /* Bold Black */
#define BOLDRED      printf("\033[1m\033[31m");      /* Bold Red */
#define BOLDGREEN    printf("\033[1m\033[32m");      /* Bold Green */
#define BOLDYELLOW   printf("\033[1m\033[33m");      /* Bold Yellow */
#define BOLDBLUE     printf("\033[1m\033[34m");      /* Bold Blue */
#define BOLDMAGENTA  printf("\033[1m\033[35m");      /* Bold Magenta */
#define BOLDCYAN     printf("\033[1m\033[36m");      /* Bold Cyan */
#define BOLDWHITE    printf("\033[1m\033[37m");      /* Bold White */
#endif

#ifndef Console
#define Console CConsole::Instance()
#endif

class CConsole
{
public:
    CConsole();
    ~CConsole() {}

public:
    static CConsole& Instance()
    {
        static CConsole consoleInst;
        return consoleInst;
    }

public:
    bool Initialize(bool colorfull=true);
    void Shutdown();

public:
    void ChangeColor();
    void ResetColor();
    void DefaultColor();
    void PrintColor(char color);

public:
    bool m_colorfull;
    char m_color_last;
    char m_color_current;
};

std::string cprintf(const char* fmt, ...);

#endif //CONSOLE_H
