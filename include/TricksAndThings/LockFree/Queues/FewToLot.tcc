#pragma once

#include "../detail/casIf.hpp"

namespace TricksAndThings { namespace LockFree { namespace detail
{

template<typename T, class C>
void FewToSingle<T, C>::Page::unRef()
{
    if (! -- used)
    {
        Storage::utilize(this);
    }
}

template<typename T, class C>
void FewToSingle<T, C>::tearOffDirtyPages()
{   // note, that here is tearing of the dirty pages
    // happens only at the 'push' way!
    if (last.template getRaw<mem::relaxed>()->back.load() < pageSize)
    {   // simple 'if(last.back < pageSize)' actially;
        // in other words - page has some free space
        // (and hence is not dirty);
        // in most cases page is not dirty indeed, so
        // instead (and before) building the PagePtr
        // (and turning up and down it's reference
        // counter) we do a fast check here;
        return;
    }

    PagePtr page;
    do
    {
        page = last.get();
    }
    while (page->back.load(mem::relaxed) == pageSize
           && last.swapIf(page->tail, [&](Page *pg1, Page *pg2)
                                      { return page == pg1 && pg2; }));
}

template<typename T, class C>
FewToSingle<T, C>::~FewToSingle()
{
    for (Page *page = first;
         page;
         page = page->tail.template getRaw<mem::relaxed>())
    {
        page->back.store(pageSize);
        page->unRef();
    }

    if (!last.template isNull<mem::relaxed>())
    {
        while (!last.get()->tail.template isNull<mem::relaxed>())
        {
            tearOffDirtyPages();
        }
    }
}

template<typename T, class C>
void FewToSingle<T, C>::init(SizeAtomic *numOfConsumers)
{
    first = last.getSubj();
    ++ *numOfConsumers;
}

template<typename T, class C>
void FewToSingle<T, C>::push(Type &&p)
{
    PagePtr keeper = last.get();

    Page *page = keeper.get();
    size_t idx;
    while (!casIf<>(page->back, [&](size_t &value)
                                {
                                    return value < pageSize ?
                                            idx = value ++ , true
                                            : false;
                                }))
    {
        page = page->tail.getSubj();
    }

    tearOffDirtyPages();

    auto entry = &page->data[idx];
    entry->itself = std::move(p);
    entry->ready.store(true, mem::release);
}

template<typename T, class C>
bool FewToSingle<T, C>::pop(Type &ret)
{
    if (first->front == pageSize)
    {
        Page *newFirst = first->tail.template getRaw<mem::acquire>();
        if (!newFirst){ return false; }
        Page *passed = first;
        first = newFirst;
        passed->unRef();
    }

    auto &entry = first->data[first->front];
    if (!entry.ready.load(mem::acquire))
    {
        return false;
    }
    ret = std::move(entry.itself);
    ++ first->front;

    return true;
}

template<typename T, class C>
size_t FewToSingle<T, C>::ClientHub::Ptr::getConsumerIdx()
{
    valueBak = ptr->consumerIdx.load();
    return valueBak;
}

template<typename T, class C>
bool FewToSingle<T, C>::ClientHub::Ptr::syncIdx(size_t *idx)
{
    if (ptr->consumerIdx.compare_exchange_strong(valueBak,
                                                 *idx))
    {
        return true;
    }
    *idx = valueBak;

    return false;
}

} } }
