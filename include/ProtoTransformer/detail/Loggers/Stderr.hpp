#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <string>
#include <sstream>

class StderrLogger
{
    enum { errorPri, warningPri, debugPri, numOf };
    std::vector<std::string> tags;

    public:

    StderrLogger() : tags(numOf)
    {
        tags[errorPri]      = "ERR";
        tags[warningPri]    = "WARN";
        tags[debugPri]      = "DBG";
    }

    int payloadCrached()    { return warningPri; }
    int errorOccured()      { return errorPri; }
    int debug()             { return debugPri; }

    void operator()(
        int priority,
        const char *fmt,
        ...)
    {
        std::stringstream newFmt;
        newFmt << tags[priority]
               << ": At " << time(0)
               <<"; pid: " << getpid()
               <<"; thread: " << pthread_self()
               << "; " << fmt << "\n";
        std::string fmtStr(newFmt.str());

        va_list params;
        va_start(params, fmtStr.c_str());
        vfprintf(stderr, fmtStr.c_str(), params);
        va_end(params);
    }
};
