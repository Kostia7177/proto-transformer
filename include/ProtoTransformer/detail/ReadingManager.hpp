#pragma once

#include "Tools.hpp"
#include "GccBug47226Satellite.hpp"

namespace ProtoTransformer
{

struct ReadingManager
{
    template<class C, typename D>
    class Itself
    {
        enum { initBufferSize = 1024 };

        public:
        typedef C Completion;
        private:
        Completion readingCompleted;

        size_t accumulated;
        size_t offset;

        typedef std::vector<D> DataBuffer;
        DataBuffer &dataBufferRef;
        DataBuffer dataRest;

        void beforeRecv();
        void afterReading(size_t);

        template<class F, typename... AdditionalCtl>
        void readAndDoSw(F f, Socket &, AdditionalCtl &&...);
        template<typename... AdditionalCtl>
        void readAndDoSw(NullType, Socket &, AdditionalCtl &&...);

        template<class F>
        class Bind
        {   // binds 'itself', 'inSocket' and 'whenCompleted' function
            // with 'readAndDoSw<Parameters...>()';
            // we cannot use 'std::bind' instead of 'Bind<F>' due to it
            // doesn't compiles with variadic templates (at least i don't
            // know how);
            GccBug47226Satellite dropClassWhenBugWillBeFixed;

            Itself *itself;
            Socket &inSocket;
            F whenCompleted;

            public:

            Bind(Itself *i, Socket &s, F f)
                 : itself(i), inSocket(s), whenCompleted(f) {}

            template<typename... AdditionalCtl>
            int operator()(AdditionalCtl &&... additionalCtl)
            {   // move readAndDo(...) call to lambda when bug will be fixed;
                itself->readAndDoSw(whenCompleted,
                                    inSocket,
                                    std::forward<AdditionalCtl>(additionalCtl)...);
                return 1;
            }
        };

        public:

        Itself(DataBuffer &);

        template<class F, typename... AdditionalCtl>
        void get(Socket &, F, AdditionalCtl &&...);
    };
};

}
#include "ReadingManager.tcc"
