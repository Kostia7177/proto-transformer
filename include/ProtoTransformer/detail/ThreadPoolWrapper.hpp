#pragma once

#include <memory>

template<class ThreadPool>
struct ThreadPoolWrapper
{
    ThreadPool itself;

    ThreadPoolWrapper(unsigned int numOfThreads) : itself(numOfThreads){}

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
struct ThreadPoolWrapper<NullType>
{
    ThreadPoolWrapper(unsigned int){}

    template<typename T> using PrivatePtr = T *;

    template<typename T> static PrivatePtr<T> getPrivate(T &arg){ return &arg; }

    template<class F> void schedule(F f){ f(); }
};
