#pragma once

#include "Tools.hpp"
#include <boost/asio.hpp>

struct ReadingManager
{
    template<class C, typename D>
    class Itself
    {
        public:
            typedef C Completion;
        private:
            enum { initBufferSize = 1024 };
            Completion readingCompleted;
            size_t accumulated;
            size_t offset;
            typedef std::vector<D> DataBuffer;
            DataBuffer &dataBufferRef;
            DataBuffer dataRest;
            template<typename SessionHdr>
            size_t readingCompletedSw(const SessionHdr &sessionHdr) { return readingCompleted(dataBufferRef, offset, accumulated, sessionHdr); }
            size_t readingCompletedSw(NullType)                     { return readingCompleted(dataBufferRef, offset, accumulated); }
            void beforeRecv()
            {
                offset = accumulated;
                if (!(dataBufferRef.size() - accumulated))
                {
                    dataBufferRef.resize(accumulated ? accumulated * 2 : initBufferSize);
                }
            }
            void afterReading(size_t size)
            {
                std::copy(dataBufferRef.begin() + size,
                          dataBufferRef.begin() + accumulated,
                          std::inserter(dataRest, dataRest.begin()));
                dataBufferRef.resize(size);
            }
            template<typename SessionHdr, class F>
            void readSw(
                boost::asio::ip::tcp::socket &inSocket,
                const SessionHdr &sessionHdr,
                F f)
            {
                if (size_t sizeOfDataCompleted = readingCompletedSw(sessionHdr))
                {
                    afterReading(sizeOfDataCompleted);
                    f();
                }
                else
                {
                    beforeRecv();
                    inSocket.async_read_some(boost::asio::buffer(&dataBufferRef[accumulated],
                                             dataBufferRef.size() - accumulated),
                                             [=, &inSocket] (const boost::system::error_code &errorCode,
                                                             size_t numOfBytes)
                                             {
                                                if (errorCode) { return; }
                                                accumulated += numOfBytes;
                                                readSw(inSocket, sessionHdr, f);
                                             });
                }
            }
            template<typename SessionHdr>
            void readSw(
                boost::asio::ip::tcp::socket &inSocket,
                const SessionHdr &sessionHdr,
                NullType)
            {
                size_t sizeOfDataCompleted;
                while (!(sizeOfDataCompleted = readingCompletedSw(sessionHdr)))
                {
                    beforeRecv();
                    accumulated += inSocket.read_some(boost::asio::buffer(&dataBufferRef[accumulated],
                                                                          dataBufferRef.size() - accumulated));
                }
                afterReading(sizeOfDataCompleted);
            }
        public:
//            ReadingManager
            Itself(DataBuffer &dataBuffer)
                : readingCompleted(Completion()),
                  accumulated(0),
                  dataBufferRef(dataBuffer){}
            template<typename SessionHdr, class F = NullType>
                void get(
                    boost::asio::ip::tcp::socket &inSocket,
                    const SessionHdr &sessionHdr,
                    F whenCompleted = F())
                {
                    dataBufferRef.clear();
                    if (!dataRest.empty()){ dataBufferRef.swap(dataRest); }

                    accumulated = dataBufferRef.size();
                    offset = 0;
                    readSw(inSocket, sessionHdr, whenCompleted);
                }
    };
};
