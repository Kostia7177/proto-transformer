#pragma once

#include "Tools.hpp"
#include "../../TricksAndThings/Tools/GccBug47226Satellite.hpp"
#include<boost/asio/spawn.hpp>

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

        void readSw(NullType, Socket &);
        void readSw(const Asio::yield_context &, Socket &);

        public:

        Itself(DataBuffer &);

        template<class F, typename... AdditionalCtl>
        void get(Socket &, F, AdditionalCtl &&...);
    };
};

}
#include "ReadingManager.tcc"
