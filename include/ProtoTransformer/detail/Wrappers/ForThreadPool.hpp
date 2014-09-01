#pragma once

#include "../Tools.hpp"
#include <memory>

namespace ProtoTransformer
{
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

}
}
