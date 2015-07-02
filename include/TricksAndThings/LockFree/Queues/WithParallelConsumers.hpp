#pragma once

#include "../detail/UsefulDefs.hpp"
#include<thread>

namespace TricksAndThings { namespace LockFree { namespace Queues
{

template<class Notepad>
class WithParallelConsumers
    : public Notepad::Cfg::InfoCalls
{   // wrapper for a pack of sub-queues, each of wich is
    // dedicated to it's consumer;
    // holds (and hides) this pack and multiplexes input
    // requests ('push') calls between the sub-queues;
    //
    Notepad notepads[Notepad::Cfg::numOfConsumersLimit];    // sub-queue pack itself;
    SizeAtomic numOfConsumers;
    typename Notepad::ClientHub clientHub;

    Notepad *selectNotepad();   // multiplexes input ('push') requests;
    Notepad *getNotepad();      // acquires the sub-queue when a consumer
                                // is initializing;

    public:

    WithParallelConsumers(size_t = 0);

    typedef typename Notepad::Type Type;

    // both pop and push are available through
    // the smart pointer-like proxies only;

    class ConsumerSideProxy
    {
        WithParallelConsumers *queue;
        Notepad *notepad;
        std::thread::id threadId;
        public:
        ConsumerSideProxy(WithParallelConsumers *);
        ConsumerSideProxy *operator->() { return this; }
        bool pop(Type &);
    };

    class ProviderSideProxy
    {
        WithParallelConsumers *queue;
        size_t idx;
        public:
        ProviderSideProxy(WithParallelConsumers *);
        ~ProviderSideProxy();
        ProviderSideProxy *operator->() { return this; }
        void push(Type &&);
//size_t
        int subQueueIdx()               { return idx; }
    };
};

} } }
#include "WithParallelConsumers.tcc"
