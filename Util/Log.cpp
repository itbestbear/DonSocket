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

#include "Log.h"
#include "Util.h"
#include "Console.h"

#include <ctime>
#include <chrono>


CLog::CLog() 
{
    m_nLevel = LOG_DEBUG;
    m_nMode  = LOG_FILE;
}

void CLog::LogProc(void* lpParameter)
{
    CLog* pthis = (CLog*)lpParameter;

    while (!pthis->m_bQuit)
    {
        pthis->LogPop();

        DonSleep(1);
    }
}

bool CLog::Initialize(const char* filename, int level, int mode)
{
    m_nMode = mode;
    m_nLevel = level;
    m_strFileName = filename;

    if(1)
    {
        m_logfp = fopen(filename, "w");
    }
    else
    {
        m_logfp = fopen(filename, "at+");
    }
    
    if (!m_logfp)
    {
        return false;
    }

    if (m_nMode == LOG_FILE)
    {
        m_logThread = std::thread(&CLog::LogProc, this, this);
    }
    
    log_trace_always("[init] log:%s level:@R%s", filename,FormatLevel(level).c_str());

    return true;
}

void CLog::Shutdown()
{
    log_trace_always("[shut] log shutdown");

    if (m_nMode == LOG_FILE)
    {
        while (m_logs.size() > 0)
        {
            DonSleep(1000);
        }

        m_bQuit = true;
        m_logThread.join();
    }
    
    if (m_logfp)
    {
        fclose(m_logfp);
    }
}

void CLog::LogPush(int level, const char *fmt, ...)
{
    if (m_nLevel > level)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    //tag 
    
    std::string strTag = FormatLevelTag(level);

    //time
    std::string strTime = "@g" + FormatCurTime();

    //msg
    char buff[512];
    va_list va;
    va_start(va, fmt);
    
    vsprintf(buff,fmt, va);
    va_end(va);
    std::string strMsg(buff);
    strMsg += "\n@T";

    std::string strLog = strTime + strTag + strMsg;
    
    //out
    if (m_nMode == LOG_FILE)
    {
        m_logs.push_back(strLog);
    }
    else
    {
        cprintf("%s",strLog.c_str());
    }
}

void CLog::LogPop()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    int nPoped = 0;
    for (int i = 0; i < m_logs.size(); i++)
    {
        std::string& strLog = m_logs[i];

        std::string data = cprintf("%s", strLog.c_str());
        FileLog((char*)data.c_str());

        nPoped++;
        if (nPoped >= 100)
            break;
    }
    if (nPoped > 0)
    {
        m_logs.erase(m_logs.begin(), m_logs.begin() + nPoped);
    }
}

void CLog::FileLog(char* log)
{
    fprintf(m_logfp, "%s", log);

    static int count = 0;
    if (count++ >= 100)
    {
        fflush(m_logfp);

        int64 nsize = ftell(m_logfp);
        if (nsize > 100 * 1024 * 1024) //100MB
        {
            fclose(m_logfp);
            m_logfp = fopen(m_strFileName.c_str(), "w");
        }
        count = 0;
    }
}

int64 CLog::GetFileSize(const char* filename)
{
    FILE* fp;
    if ((fp = fopen(filename, "r")) == NULL)
        return 0;
    fseek(fp, 0, SEEK_END);
    int64 nsize = ftell(fp);
    fclose(fp);

    return nsize;
}

std::string CLog::FormatLevel(int nLevel)
{
    switch (nLevel)
    {
    case LOG_DEBUG: return "LOG_DEBUG";
    case LOG_INFO:  return "LOG_INFO";
    case LOG_WARN:  return "LOG_WARN";
    case LOG_ERROR: return "LOG_ERROR";
    case LOG_FATAL: return "LOG_FATAL";
    case LOG_ALWAYS:return "LOG_ALWAYS";
    default:
        break;
    }

    return "Unknown";
}

std::string CLog::FormatLevelTag(int nLevel)
{
    char buf[64];
    switch (nLevel)
    {
    case LOG_DEBUG: snprintf(buf, sizeof(buf), "%-*s", 6, "@W[DEBUG]"); break;
    case LOG_INFO:  snprintf(buf, sizeof(buf), "%-*s", 6, "@G[INFO]"); break;
    case LOG_WARN:  snprintf(buf, sizeof(buf), "%-*s", 6, "@M[WARN]"); break;
    case LOG_ERROR: snprintf(buf, sizeof(buf), "%-*s", 6, "@R[ERROR]"); break;
    case LOG_FATAL: snprintf(buf, sizeof(buf), "%-*s", 6, "@Y[FATAL]"); break;
    case LOG_ALWAYS:snprintf(buf, sizeof(buf), "%s", "@C"); break;
    default:
        snprintf(buf, sizeof(buf), "%-*s", 6, "@R[Unknown]"); break;
        break;
    }

    return buf;
}

void CLog::ChangeLevel(std::vector<std::string> cmds)
{
    if (cmds.size() != 2)
    {
        cprintf("@Rwrong command,eg:l 2\n@T");
        return;
    }

    int newlevel = atoi(cmds[1].c_str());
    if (newlevel<LOG_DEBUG || newlevel>=LOG_MAX)
    {
        cprintf("@Rnew level is out of range:[%d-%d)\n@T", LOG_DEBUG, LOG_MAX);
        return;
    }

    m_nLevel = newlevel;

    cprintf("@Clog level is changed to [%s]\n@T", FormatLevel(m_nLevel).c_str());
}



