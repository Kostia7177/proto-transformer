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

struct GetRequestSize
{
    uint32_t operator()(const AnyHdr &hdr)const{ return getHdrField(hdr, sizeFieldIdx); }
};

struct SetRequestSize
{
    void operator()(uint32_t size, AnyHdr &hdr)const{ setHdrField(size, hdr, sizeFieldIdx); }
};
