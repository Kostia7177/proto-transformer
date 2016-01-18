#pragma once

#include<memory>

namespace ProtoTransformer
{

template<class T, class Base = NullType>
struct SessionThreadPoolIs : virtual Base
{
    class TaskManager
    {
        public:
        typedef T Itself;
        private:
        Itself itself;

        public:

        TaskManager(unsigned int numOfThreads) : itself(numOfThreads){}

        template<typename Data> using PrivatePtr = std::shared_ptr<Data>;

        template<typename Data> static PrivatePtr<Data> getPrivate(Data &arg)
        {
            PrivatePtr<Data> retPtr(new Data);
            retPtr->Swap(arg, Int2Type<Data::serverSendsAnswer>());
            return retPtr;
        }

        template<class F> void schedule(F f){ itself.schedule(f); }
    };

    enum { proto = 0 };
};

template<class Base>
struct SessionThreadPoolIs<NullType, Base> : virtual Base
{
    struct TaskManager
    {
        typedef NullType Itself;

        TaskManager(unsigned int){}

        template<typename T> using PrivatePtr = T *;

        template<typename T> static PrivatePtr<T> getPrivate(T &arg){ return &arg; }

        template<class F> void schedule(F f){ f(); }
    };

    enum { proto = 0 };
};

}
