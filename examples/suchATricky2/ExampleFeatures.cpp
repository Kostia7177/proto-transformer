#include <arpa/inet.h>
#include "ExampleFeatures.hpp"

uint32_t getHdrField(
    const AnyHdr &hdr,
    HdrFieldIndex idx)
{
    uint32_t ret = *(uint32_t *)(hdr.itself + sizeOfField * idx);
    return ntohl(ret);
}

void setHdrField(
    uint32_t value,
    AnyHdr &hdr,
    HdrFieldIndex idx)
{
    uint32_t *fieldPtr = (uint32_t *)(hdr.itself + sizeOfField * idx);
    *fieldPtr = htonl(value);
}
