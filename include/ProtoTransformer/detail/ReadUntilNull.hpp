#pragma once

#include <vector>

class ReadUntilNull
{
    typedef std::vector<char> DataBuffer;

    size_t doIt(const DataBuffer &dataBuffer, size_t offset, size_t numOfBytes)
    {
        if (!numOfBytes) { return 0; }
        typedef DataBuffer::value_type DataRepr;
        DataRepr *endOfStr = (DataRepr *)memchr(&dataBuffer[offset], 0, numOfBytes - offset);
        if (endOfStr){ return (endOfStr + 1) - &dataBuffer[0]; }
        return 0;
    }
    public:

    template<typename SessionHdr>
    size_t operator()(const DataBuffer &dataBuffer, size_t offset, size_t numOfBytes, const SessionHdr &)
    {
        return doIt(dataBuffer, offset, numOfBytes);
    }

    size_t operator()(const DataBuffer &dataBuffer, size_t offset, size_t numOfBytes)
    {
        return doIt(dataBuffer, offset, numOfBytes);
    }
};
