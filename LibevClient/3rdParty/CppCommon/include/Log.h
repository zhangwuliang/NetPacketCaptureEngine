#ifndef __LOG_H__
#define __LOG_H__

#include <string>

#include "log4cxx/logger.h"
#include "log4cxx/logstring.h"
#include "log4cxx/propertyconfigurator.h"

enum LogLevel
{
    TRANCE = 0,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
};

class Loger 
{

public:

    Loger();
    ~Loger();
    static void  Log(LogLevel level, const char* msgfmt, ...);
    static void  Log(const char* logName, LogLevel level, const char* msgfmt, ...);

    /**
     * init Log4cxx with configure file
     */
    static void Initialize(const char* propertyConfigFile, const char* projectName);
    static bool IsEnableDebugLog();


};


#endif /*__LOG_H__*/

