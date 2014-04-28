#pragma once

#include "Tools.hpp"
#include <boost/asio.hpp>

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

        template<typename AdditionalCtl>
        size_t readingCompletedSw(const AdditionalCtl &additionalCtl)   { return readingCompleted(dataBufferRef, offset, accumulated, additionalCtl); }
        size_t readingCompletedSw(NullType)                             { return readingCompleted(dataBufferRef, offset, accumulated); }

        void beforeRecv();
        void afterReading(size_t);

        template<class F, typename AdditionalCtl>
        void readAndDoSw(F f, boost::asio::ip::tcp::socket &, const AdditionalCtl &);
        template<typename AdditionalCtl>
        void readAndDoSw(NullType, boost::asio::ip::tcp::socket &, const AdditionalCtl &);

        public:

        Itself(DataBuffer &);

        template<typename AdditionalCtl, class F = NullType>
        void get(boost::asio::ip::tcp::socket &, const AdditionalCtl &, F = F());
    };
};

}
#include "ReadingManager.tcc"
