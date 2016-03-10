#include "../../TricksAndThings/SignatureManip/filteringAdapter.hpp"

namespace ProtoTransformer
{

template<class C, typename D>
ReadingManager::Itself<C, D>::Itself(DataBuffer &dataBuffer)
    : readingCompleted(Completion()),
      accumulated(0),
      dataBufferRef(dataBuffer)
{
}

template<class C, typename D>
void ReadingManager::Itself<C, D>::beforeRecv()
{
    offset = accumulated;

    if (!(dataBufferRef.size() - accumulated))
    {
        dataBufferRef.resize(accumulated ?
                                accumulated * 2
                                : initBufferSize);
    }
}

template<class C, typename D>
void ReadingManager::Itself<C, D>::afterReading(size_t size)
{
    std::copy(dataBufferRef.begin() + size,
              dataBufferRef.begin() + accumulated,
              std::inserter(dataRest, dataRest.begin()));

    dataBufferRef.resize(size);
}

template<class C, typename D>
void ReadingManager::Itself<C, D>::readSw(
    NullType,
    Socket &inSocket)
{
    accumulated += inSocket.read_some(Asio::buffer(&dataBufferRef[accumulated],
                                                   dataBufferRef.size() - accumulated));
}

template<class C, typename D>
void ReadingManager::Itself<C, D>::readSw(
    const Asio::yield_context &yield,
    Socket &inSocket)
{
    accumulated += inSocket.async_read_some(Asio::buffer(&dataBufferRef[accumulated],
                                                         dataBufferRef.size() - accumulated),
                                            yield);
}

template<class C, typename D>
template<class Y, typename... AdditionalCtl>
void ReadingManager::Itself<C, D>::get(
    Socket &inSocket,
    Y maybeYield,
    AdditionalCtl &&... additionalCtl)
{
    dataBufferRef.clear();
    if (!dataRest.empty()){ dataBufferRef.swap(dataRest); }

    accumulated = dataBufferRef.size();
    offset = 0;
    size_t sizeOfDataCompleted;
    while (!(sizeOfDataCompleted = TricksAndThings::
                                   filteringAdapter(readingCompleted,
                                                    static_cast<const DataBuffer &>(dataBufferRef),
                                                    static_cast<const size_t &>(offset),
                                                    static_cast<const size_t &>(accumulated),
                                                    std::forward<AdditionalCtl>(additionalCtl)...)))
    {
        beforeRecv();
        readSw(maybeYield, inSocket);
    }

    afterReading(sizeOfDataCompleted);
}

}
