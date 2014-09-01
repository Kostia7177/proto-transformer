#pragma once

#include <vector>

struct ReadUntilNull
{
    typedef std::vector<char> DataBuffer;

    int operator()(const DataBuffer &dataBuffer, const size_t &offset, const size_t &numOfBytes)
    {
        if (!numOfBytes) { return 0; }

        typedef DataBuffer::value_type DataRepr;
        DataRepr *endOfStr = (DataRepr *)memchr(&dataBuffer[offset], 0, numOfBytes - offset);
        if (endOfStr){ return (endOfStr + 1) - &dataBuffer[0]; }

        return 0;
    }
};
