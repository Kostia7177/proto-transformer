#pragma once

#include<atomic>
#include<cstddef>

namespace TricksAndThings { namespace LockFree
{

enum { dwordSize = sizeof(void *) * 2 };

struct mem
{   // ...is just a shortcut for an 'std::memory_order[...]'
    typedef std::memory_order Order;
    static const Order relaxed = std::memory_order_relaxed;
    static const Order acquire = std::memory_order_acquire;
    static const Order release = std::memory_order_release;
};

template<mem::Order l, mem::Order s>
struct CasMode
{
    static const mem::Order loading = l;
    static const mem::Order storing = s;
};

typedef CasMode<mem::relaxed, mem::acquire> AsConsumer;
typedef CasMode<mem::relaxed, mem::acquire> AsProducer;
typedef CasMode<mem::relaxed, mem::relaxed> Free;

typedef std::atomic<size_t> SizeAtomic;
} }
