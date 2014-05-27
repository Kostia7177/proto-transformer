#pragma once

#include <syslog.h>
#include <stdarg.h>

struct SyslogLogger
{
    int payloadCrached()    { return LOG_WARNING; }
    int errorOccured()      { return LOG_ERR; }
    int debug()             { return LOG_DEBUG; }

    void operator()(
        int priority,
        const char *fmt,
        ...)
    {
        va_list params;
        va_start(params, fmt);
        vsyslog(priority, fmt, params);
        va_end(params);
    }
};
