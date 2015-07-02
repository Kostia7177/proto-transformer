#include<algorithm>
#include<stdexcept>
#include<sstream>

namespace TricksAndThings { namespace LockFree { namespace Queues
{

template<class Notepad>
Notepad *WithParallelConsumers<Notepad>::selectNotepad()
{
    if (!numOfConsumers) { return 0; }

    typename Notepad::ClientHub::Ptr clientHubPtr = &clientHub;
    size_t idx = clientHubPtr->getConsumerIdx();
    Notepad *ptr = 0;
    do
    {
        ptr = &notepads[idx ++ ];
        if (!notepads[idx].ready()) { idx = 0; }
    }
    while (!clientHubPtr->syncIdx(&idx));

    return ptr;
}

template<class Notepad>
Notepad *WithParallelConsumers<Notepad>::getNotepad()
{
    for (size_t idx = 0; idx < Notepad::Cfg::numOfConsumersLimit; ++ idx)
    {
        Notepad *ptr = &notepads[idx];
        if (ptr->acquire())
        {
            if (!ptr->ready()){ ptr->init(&numOfConsumers); }
            return ptr;
        }
    }

    return 0;
}

template<class Notepad>
WithParallelConsumers<Notepad>::WithParallelConsumers(size_t n)
    : numOfConsumers(0)
{
    if (numOfConsumers > sizeof(notepads) / sizeof(Notepad))
    {
        std::stringstream exc;
        exc << "Number of consumers cannot be greater than";
        throw std::runtime_error(exc.str());
    }

    std::for_each(notepads, notepads + n, [&](Notepad &notepad)
                                          { notepad.init(&numOfConsumers); });
}

template<class Notepad>
WithParallelConsumers<Notepad>::ConsumerSideProxy::ConsumerSideProxy(WithParallelConsumers *q)
    : queue(q),
      threadId(std::this_thread::get_id())
{
    if (!queue)
    {
        throw std::runtime_error("Consumer side proxy pointer "
                                 "to a parallel-consumers queue "
                                 "cannot be initialized by zero!");
    }

    notepad = queue->getNotepad();
}

template<class Notepad>
bool WithParallelConsumers<Notepad>::ConsumerSideProxy::pop(Type &p)
{
    std::thread::id caller = std::this_thread::get_id();
    if (threadId != caller)
    {
        std::stringstream exc;
        exc << "Queue consumer, owned by thread id " << threadId
            << ", was called from thread id " << caller
            << "; ";
        throw std::runtime_error(exc.str());
    }

    bool ret = notepad->pop(p);
    if (ret) { queue->decrSize(); }

    return ret;
}

template<class Notepad>
WithParallelConsumers<Notepad>::ProviderSideProxy::ProviderSideProxy(WithParallelConsumers *q)
    : queue(q)
{
    if (!queue)
    {
        throw std::runtime_error("Provider side proxy pointer "
                                 "to a parallel-comsumers queue "
                                 "cannot be initialized by zero!");
    }

    queue->clientHub.onNewProvider();
}

template<class Notepad>
WithParallelConsumers<Notepad>::ProviderSideProxy::~ProviderSideProxy()
{
    queue->clientHub.onProviderExited();
}

template<class Notepad>
void WithParallelConsumers<Notepad>::ProviderSideProxy::push(Type &&p)
{
    if (Notepad *notepad = queue->selectNotepad())
    {
        notepad->push(std::forward<Type>(p));
        queue->incrSize();
    }
    else { throw std::runtime_error("No initialized sub-queue found!"); }
}

} } }
