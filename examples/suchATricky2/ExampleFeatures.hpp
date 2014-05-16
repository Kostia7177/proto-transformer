#pragma once

#include <cstddef>

enum { sumNumbers, sortNumbers, terminateSession, logRequest, echo, numOfOpts };
enum HdrFieldIndex { pauseFieldIdx, sizeFieldIdx, optFieldIdx, numOfFields };
enum { sizeOfField = 32 };

struct AnyHdr
{
    unsigned char itself[sizeOfField * numOfFields];
};

struct Answer
{
    char data[1024];
};

uint32_t getHdrField(const AnyHdr &, HdrFieldIndex);
void setHdrField(uint32_t, AnyHdr &, HdrFieldIndex);

struct AnyHdrWrapped
{
    typedef AnyHdr Itself;

    static uint32_t getSize(const AnyHdr &hdr){ return getHdrField(hdr, sizeFieldIdx); }
    static void setSize2(uint32_t size, AnyHdr &hdr){ setHdrField(size, hdr, sizeFieldIdx); }
};
