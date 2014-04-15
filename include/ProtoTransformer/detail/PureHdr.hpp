#pragma once

#include <arpa/inet.h>

namespace ProtoTransformer
{

// the simplest (pure) header that doesn't carries anything but size
// of a request or of an answer; filling and scanning automatically,
// i.e. transparent for user's code; not to be passed to it;
typedef uint32_t PureHdr;

struct Network2HostLong
{
    uint32_t operator()(PureHdr hdr) const       { return ntohl(hdr); }
};

struct Host2NetworkLong
{
    void operator()(uint32_t size, PureHdr &hdr) { hdr = htonl(size); }
};

}
