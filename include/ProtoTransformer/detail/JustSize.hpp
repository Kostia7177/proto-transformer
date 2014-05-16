#pragma once

#include <arpa/inet.h>

namespace ProtoTransformer
{

// the simplest (pure) header that doesn't carries anything but size
// of a request or of an answer; filling and scanning automatically,
// i.e. transparent for user's code; not to be passed to it;
struct JustSize
{
    typedef uint32_t Itself;

    static uint32_t getSize(Itself hdr)                 { return ntohl(hdr); }
    static void setSize2(uint32_t size, Itself &hdr)    { hdr = htonl(size); }
};

}
