#include <stdarg.h>
#include <stdio.h>

#include "BaseLock.h"
#include "Log.h"

static BaseLock                      g_logLock;

static const unsigned int BUFFERSIZE = 4096;
static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("bslog"));
#define msgFormatA(buf, msg) \
        do { \
            va_list ap; \
            va_start(ap, msg); \
            vsprintf(buf, msg, ap); \
            va_end(ap); \
        } while (0)

Loger::Loger()
{
}

Loger::~Loger()
{
}

void  Loger::Log(LogLevel level, const char* msgfmt, ...)
{
    BSLock  bsLock(g_logLock);

    char buf[BUFFERSIZE] = {0};
    msgFormatA(buf, msgfmt);
    switch (level)
	{
        case TRANCE:
            //Trace(msgfmt);
            LOG4CXX_TRACE(logger, buf);
            break;
        case DEBUG:
            LOG4CXX_DEBUG(logger, buf);
            break;
        case INFO:
            LOG4CXX_INFO(logger, buf);
            break;
        case WARNING:
            LOG4CXX_WARN(logger, buf);
            break;
        case ERROR:
            LOG4CXX_ERROR(logger, buf);
            break;
        default:
            break;
    }
}


void  Loger::Log(const char* logName, LogLevel level, const char* msgfmt, ...)
{
}

void Loger::Initialize(const char* propertyConfigFile, const char* projectName)
{
    log4cxx::PropertyConfigurator::configure(propertyConfigFile);
    logger = log4cxx::Logger::getLogger(projectName);
}

bool Loger::IsEnableDebugLog()
{
    return true;
}


