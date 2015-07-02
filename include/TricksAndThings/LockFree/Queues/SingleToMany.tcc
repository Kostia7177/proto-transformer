#pragma once

#include<stdexcept>

namespace TricksAndThings { namespace LockFree { namespace detail
{

template<typename T, class C>
bool SingleToSingle<T, C>::tearOffFirstPage()
{
    if (!first) { return false; }

    Page *passed = first;
    first = first->tail.load();
    Storage::utilize(passed);

    return true;
}

template<typename T, class C>
void SingleToSingle<T, C>::init(SizeAtomic *numOfConsumers)
{
    first = Storage::make();
    last = first;
    ++ *numOfConsumers;
}

template<typename T, class C>
void SingleToSingle<T, C>::push(Type &&p)
{
    last->data[last->back] = std::move(p);

    auto &backBak = last->back;
    if (last->back == pageSize - 1)
    {   // ...there is no more space on the
        // last page - so, create a new one
        // as it's tail...
        last->tail.store(Storage::make());
        // ...and point to it;
        last = last->tail;
    }

    backBak.fetch_add(1);   // now the reader
                            // can see the
                            // new-pushed data;
}

template<typename T, class C>
bool SingleToSingle<T, C>::pop(Type &ret)
{
    size_t idx = first->front;
    if (idx < first->back.load())
    {   // ...there is at least one ready
        // to pop element at the page;
        ret = std::move(first->data[idx]);
        if ( ++ first->front == pageSize)
        {   // ...page is exhausted;
            tearOffFirstPage();
        }

        return true;
    }

    return false;
}

template<typename T, class C>
void SingleToSingle<T, C>::ClientHub::onNewProvider()
{
    if (hasProvider.test_and_set())
    {
        throw std::runtime_error("Acquiring the single-provider queue by "
                                 "more than one provider is not allowed!");
    }
}

} } }
