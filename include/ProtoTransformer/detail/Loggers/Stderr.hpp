#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <string>

class StderrLogger
{
    enum { errorPri, warningPri, debugPri, numOf };
    std::vector<std::string> tags;

    public:

    StderrLogger()
    {
    }

    int payloadCrached()    { return warningPri; }
    int errorOccured()      { return errorPri; }
    int debug()             { return debugPri; }

    void operator()(
        int priority,
        const char *fmt,
        ...)
    {
        va_list params;
        va_start(params, fmt);
        vfprintf(stderr, fmt, params);
        va_end(params);
    }
};
