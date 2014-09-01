#pragma once

#include "../Tools.hpp"
#include <stdarg.h>

namespace ProtoTransformer
{
namespace Wrappers
{

template<class T>
struct ForLogger
{
    typedef T Itself;
    Itself itself;
};

template<>
struct ForLogger<NullType>
{
    struct Itself
    {
        void operator()(int, const char *, ...){}
        int payloadCrached()    { return 0; }
        int errorOccured()      { return 0; }
        int debug()             { return 0; }
    };

    Itself itself;
};

}
}
