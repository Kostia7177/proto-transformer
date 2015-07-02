#pragma once

#include "../detail/UsefulDefs.hpp"
#include "../detail/NotepadBase.hpp"
#include "WithParallelConsumers.hpp"
#include "Traits.hpp"

namespace TricksAndThings { namespace LockFree
{
namespace detail
{

template<typename T, class C>
class SingleToSingle
    : public NotepadBase<C>
{   // the one-provider and one-consumer queue implementation;
    // used as a sub-queue (one of many parallel sub-queues)
    // within a 'SingleToMany' instantiation of a
    // 'WithParallelConsumers' queue;
    public:
    typedef T Type;
    typedef C Cfg;
    private:

    static const size_t pageSize = Cfg::pageSize;

    struct Page
    {
        Type data[pageSize];

        size_t front;               // -- index of the first element that is
                                    // available for reading;
        SizeAtomic back;            // -- index of the first free (available
                                    // for writing) cell;
                                    // note that there's no concurrency on both
                                    // 'back' and 'tail' indeed - atomic types
                                    // needed to support a memory access order;
        std::atomic<Page *> tail;   // -- next page;

        Page() : front(0), back(0), tail(0) {}
    };

    typedef typename Cfg::template Storage<Page> Storage;

    Page *first;    // operated by reading thread (pop way);
    Page *last;     // operated by writing thread (push way);

    bool tearOffFirstPage();    // reader calls this when there
                                // is no data left on the page,
                                // also dtor calls it to drop
                                // all the pages the object
                                // contains;

    public:

    SingleToSingle() : first(0), last(0) {}
    ~SingleToSingle()   { while (tearOffFirstPage()); }

    void init(SizeAtomic *);
    bool ready()        { return first; }
    void push(Type &&);
    bool pop(Type &);

    class ClientHub
    {   // a detail that is to be exported to a holding instantiation
        // of a 'WithParallelConsumers' queue;
        size_t consumerIdx;             // a sub-queue multiplexer, used by
                                        // a writing thread to remember the
                                        // last sub-queue that was pushed;
        std::atomic_flag hasProvider;   // used to prevent a re-acquisition
                                        // of the queue that is already
                                        // acquired by any provider;

        public:

        ClientHub() : consumerIdx(0), hasProvider(ATOMIC_FLAG_INIT) {}

        typedef ClientHub *Ptr;

        size_t getConsumerIdx()     { return consumerIdx; }
        bool syncIdx(size_t *idx)   { return consumerIdx = *idx, true; }
        void onNewProvider();
        void onProviderExited()     { hasProvider.clear(); }
    };
};

} // <-- namespace detail

namespace Queues
{
template<typename T, class Cfg = Traits<>> using SingleToMany = WithParallelConsumers<detail::SingleToSingle<T, Cfg>>;
}

} }
#include "SingleToMany.tcc"
