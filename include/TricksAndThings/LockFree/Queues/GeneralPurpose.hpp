#pragma once

#include "Traits.hpp"
#include "../detail/Entry.hpp"
#include "../Tools/ObjHolder.hpp"
#include "../detail/UsefulDefs.hpp"
#include "../../AtomicAccessedAny.hpp"

namespace TricksAndThings { namespace LockFree { namespace Queues
{

template<typename T, class Cfg = Traits<>>
class GeneralPurpose
    : public Cfg::InfoCalls
{
    static const size_t pageSize = Cfg::pageSize;

    enum
    {
        pushWay,
        popWay,
        numOfWays,

        back = pushWay,
        front = popWay
    };

    struct Page
    {
        typedef ObjHolder<Page, typename Cfg::template Storage<Page>> Holder;

        detail::Entry<T> data[pageSize];

        Holder tail[numOfWays];
        SizeAtomic pointers[numOfWays];
        std::atomic<uint64_t> used;

        Page();

        void refOn()    { ++ used; }
        void unRef();
        void clear();
    };

    typedef typename Cfg::template Storage<Page> Storage;

    typedef typename Page::Holder Holder;
    Holder notepad[numOfWays];

    typedef typename Holder::Ptr PagePtr;

    template<size_t> void tearOffDirtyPages();
    template<size_t> void clear();

    public:

    GeneralPurpose(){}
    ~GeneralPurpose();

    void push(T &&);
    bool pop(T &);
};

} } }
#include "GeneralPurpose.tcc"
