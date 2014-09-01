#include "ParamPackManip/filteringAdapter.hpp"

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
template<class F, typename... AdditionalCtl>
void ReadingManager::Itself<C, D>::readAndDoSw(
    F f,
    Socket &inSocket,
    AdditionalCtl &&... additionalCtl)
{
    if (size_t sizeOfDataCompleted = filteringAdapter(readingCompleted,
                                                      static_cast<const DataBuffer &>(dataBufferRef),
                                                      static_cast<const size_t &>(offset),
                                                      static_cast<const size_t &>(accumulated),
                                                      std::forward<AdditionalCtl>(additionalCtl)...))
    {
        afterReading(sizeOfDataCompleted);
        f();
    }
    else
    {
        beforeRecv();

        GccBug47226Satellite bugOverriding;
        // drop the 'paramsHierarchized' when the g++ bug will be fixed;
        Params2Hierarchy<BindNotNullsOnly,  AdditionalCtl...> paramsHierarchized(additionalCtl...);
        GccBug47226Satellite endOfBugOverriding;

        inSocket.async_read_some(Asio::buffer(&dataBufferRef[accumulated],
                                              dataBufferRef.size() - accumulated),
                                 [=, &inSocket] (const Sys::error_code &errorCode, size_t numOfBytes)
                                 {
                                    if (errorCode) { return; }
                                    accumulated += numOfBytes;

                                    GccBug47226Satellite bugOverriding1;
                                    // drop the 'bind' and...
                                    Bind<F> bind(this, inSocket, f);
                                    // ...replace the following call with the Bind::operator()(...)'s body...
                                    Hierarchy2Params<decltype(paramsHierarchized)>::call(bind,
                                                                                         paramsHierarchized);
                                    // ...when the g++ bug will be fixed;
                                    GccBug47226Satellite endOfBugOverriding1;
                                 });
    }
}

template<class C, typename D>
template<typename... AdditionalCtl>
void ReadingManager::Itself<C, D>::readAndDoSw(
    NullType,
    Socket &inSocket,
    AdditionalCtl &&... additionalCtl)
{
    size_t sizeOfDataCompleted;
    while (!(sizeOfDataCompleted = filteringAdapter(readingCompleted,
                                                    static_cast<const DataBuffer &>(dataBufferRef),
                                                    static_cast<const size_t &>(offset),
                                                    static_cast<const size_t &>(accumulated),
                                                    std::forward<AdditionalCtl>(additionalCtl)...)))
    {
        beforeRecv();
        accumulated += inSocket.read_some(Asio::buffer(&dataBufferRef[accumulated],
                                                       dataBufferRef.size() - accumulated));
    }
    afterReading(sizeOfDataCompleted);
}

template<class C, typename D>
template<class F, typename... AdditionalCtl>
void ReadingManager::Itself<C, D>::get(
    Socket &inSocket,
    F whenCompleted,
    AdditionalCtl &&... additionalCtl)
{
    dataBufferRef.clear();
    if (!dataRest.empty()){ dataBufferRef.swap(dataRest); }

    accumulated = dataBufferRef.size();
    offset = 0;
    readAndDoSw(whenCompleted, inSocket, additionalCtl...);
}

}
