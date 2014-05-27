#pragma once

#include "Tools.hpp"
#include <memory>
#include <stdarg.h>

namespace Wrappers
{

template<class ThreadPool>
struct ForThreadPool
{
    ThreadPool itself;

    ForThreadPool(unsigned int numOfThreads) : itself(numOfThreads){}

    template<typename Data> using PrivatePtr = std::shared_ptr<Data>;

    template<typename Data> static PrivatePtr<Data> getPrivate(Data &arg)
    {
        PrivatePtr<Data> retPtr(new Data);
        retPtr->Swap(arg, Int2Type<Data::serverSendsAnswer>());
        return retPtr;
    }

    template<class F> void schedule(F f){ itself.schedule(f); }
};

template<>
struct ForThreadPool<NullType>
{
    ForThreadPool(unsigned int){}

    template<typename T> using PrivatePtr = T *;

    template<typename T> static PrivatePtr<T> getPrivate(T &arg){ return &arg; }
    template<class F> void schedule(F f){ f(); }
};

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

template<class T>
struct ForDataHeader
{
    typedef T Type;
    static_assert(IsTransferable<typename Type::Itself>::value == true,
                  "Request or answer data header must be either arithmetic, or pod. "
                  "Or NullType.");
};

template<>
struct ForDataHeader<NullType>
{
    struct Type
    {
        typedef NullType Itself;

        static uint32_t getSize(Itself)             { return 0; }
        static void setSize2(uint32_t, Itself &)    {}
    };
};

}
