#pragma once

#include "../detail/UsefulDefs.hpp"

namespace TricksAndThings { namespace LockFree
{

template<class P>
class RecyclingStorage
{
    class RawBuf;
    struct RevisedPtr
    {
        RawBuf *ptr;
        uint64_t rev;
        RevisedPtr() noexcept: ptr(0), rev(0){}
    };

    class RawBuf
    {
        unsigned char padding[sizeof(P)];

        public:

        RawBuf(RawBuf *p)
        { *nextPtr() = p; }

        RawBuf **nextPtr()
        { return reinterpret_cast<RawBuf **>(&padding); }
    };

    struct AtomicRevisedPtr
    {
        std::atomic<RevisedPtr> itself;
        ~AtomicRevisedPtr(){ cleanup(); }
    };

    static AtomicRevisedPtr data;

    static void cleanup();

    public:

    template<typename... Args>
    static P *make(Args &&...);

    static void utilize(P *);
};

template<class P> alignas(dwordSize) typename RecyclingStorage<P>::AtomicRevisedPtr RecyclingStorage<P>::data;

} }
#include "RecyclingStorage.tcc"
