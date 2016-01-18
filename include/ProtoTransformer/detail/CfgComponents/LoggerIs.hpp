#pragma once

#include "../Tools.hpp"

namespace ProtoTransformer
{

template<class T, class Base = NullType>
struct LoggerIs : virtual Base
{
    typedef T Logger;
    enum { proto = 0 };
};

template<class Base>
struct LoggerIs<NullType, Base> : virtual Base
{
    struct Logger
    {
        void operator()(int, const char *, ...){}
        int payloadCrached()    { return 0; }
        int errorOccured()      { return 0; }
        int debug()             { return 0; }
    };

    enum { proto = 0 };
};

}
