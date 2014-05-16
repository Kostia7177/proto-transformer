#pragma once

#include <time.h>
#include <cstdint>
#include <arpa/inet.h>

struct AnySessionHdr
{
    unsigned char itself[512];
};

static inline uint32_t *getNum(const AnySessionHdr &hdr)      { return (uint32_t *)hdr.itself; }
static inline char *getName(const AnySessionHdr &hdr)         { return (char *)(hdr.itself + sizeof(uint32_t)); }
static inline size_t getNameSize(const AnySessionHdr &hdr)    { return sizeof(hdr.itself) - sizeof(uint32_t); }

struct AnySessionSpecific
{
    uint32_t numOfRequestsLeft;
    time_t startedAt;
};

struct InitAnySessionSpecific
{
    int operator() (const AnySessionHdr &, AnySessionSpecific &);
};

struct AnyAnswerHdr
{
    unsigned char itself[64];
};

static inline uint32_t *getNumPtr(const AnyAnswerHdr &hdr) { return (uint32_t *)hdr.itself; }

struct AnyAnswerHdrWrapped
{
    typedef AnyAnswerHdr Itself;

    static uint32_t getSize(const AnyAnswerHdr &answerHdr){ return ntohl(*(uint32_t *)(answerHdr.itself + sizeof(uint32_t))); }
    static void setSize2(uint32_t size, AnyAnswerHdr &answerHdr){ *(uint32_t *)(answerHdr.itself + sizeof(uint32_t)) = htonl(size); }
};
