#pragma once

#include<atomic>

namespace TricksAndThings { namespace LockFree { namespace detail
{

template<typename T>
struct Entry
{
    T itself;
    std::atomic<bool> ready;

    Entry() : ready(false){}
};

} } }
