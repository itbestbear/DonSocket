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

#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdarg.h>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

/// 定义日志等级
enum
{
    LOG_DEBUG = 1,
    LOG_INFO  = 2,
    LOG_WARN  = 3,
    LOG_ERROR = 4,
    LOG_FATAL = 5,
    LOG_ALWAYS = 6,

    LOG_MAX,
};

enum
{
    LOG_CONSOLE,    //log to console
    LOG_FILE,       //log to console and file
};

#ifndef int64
typedef long long int64;
#endif

#ifndef Log
#define Log CLog::Instance()
#endif

class CLog
{
public:
    CLog();
    ~CLog() {}
public:
    static CLog& Instance()
    {
        static CLog logInst;
        return logInst;
    }
private:
    void LogProc(void* lpParameter);

public:
    bool Initialize(const char* filename,int level=LOG_DEBUG,int mode=LOG_CONSOLE);
    void Shutdown();
public:
    void LogPush(int level,const char* fmt, ...);
    void LogPop();
    void SetLevel(int level) { m_nLevel = level; }
    void FileLog(char* log);
    void ChangeLevel(std::vector<std::string> cmds);
private:
    int64  GetFileSize(const char* filename);
    std::string FormatLevel(int nLevel);
    std::string FormatLevelTag(int nLevel);
private:
    std::string m_strFileName;
    int         m_nLevel;
    FILE*       m_logfp = nullptr;
    int         m_nMode;
    
private:
    std::mutex  m_mutex;
    std::thread	m_logThread;
    bool        m_bQuit=false;
    std::vector<std::string> m_logs;
};

#define log_trace_debug(...)  Log.LogPush(LOG_DEBUG,__VA_ARGS__)
#define log_trace_info(...)   Log.LogPush(LOG_INFO,__VA_ARGS__)
#define log_trace_warn(...)   Log.LogPush(LOG_WARN,__VA_ARGS__)
#define log_trace_error(...)  Log.LogPush(LOG_ERROR,__VA_ARGS__)
#define log_trace_fatal(...)  Log.LogPush(LOG_FATAL,__VA_ARGS__)
#define log_trace_always(...) Log.LogPush(LOG_ALWAYS,__VA_ARGS__)



#endif //LOG_H
