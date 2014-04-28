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
template<class F, typename AdditionalCtl>
void ReadingManager::Itself<C, D>::readAndDoSw(
    F f,
    boost::asio::ip::tcp::socket &inSocket,
    const AdditionalCtl &additionalCtl)
{
    if (size_t sizeOfDataCompleted = readingCompletedSw(additionalCtl))
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
                                    readAndDoSw(f, inSocket, additionalCtl);
                                 });
    }
}

template<class C, typename D>
template<typename AdditionalCtl>
void ReadingManager::Itself<C, D>::readAndDoSw(
    NullType,
    boost::asio::ip::tcp::socket &inSocket,
    const AdditionalCtl &additionalCtl)
{
    size_t sizeOfDataCompleted;
    while (!(sizeOfDataCompleted = readingCompletedSw(additionalCtl)))
    {
        beforeRecv();
        accumulated += inSocket.read_some(boost::asio::buffer(&dataBufferRef[accumulated],
                                                              dataBufferRef.size() - accumulated));
    }
    afterReading(sizeOfDataCompleted);
}

template<class C, typename D>
template<typename AdditionalCtl, class F>
void ReadingManager::Itself<C, D>::get(
    boost::asio::ip::tcp::socket &inSocket,
    const AdditionalCtl &additionalCtl,
    F whenCompleted)
{
    dataBufferRef.clear();
    if (!dataRest.empty()){ dataBufferRef.swap(dataRest); }

    accumulated = dataBufferRef.size();
    offset = 0;
    readAndDoSw(whenCompleted, inSocket, additionalCtl);
}

}
