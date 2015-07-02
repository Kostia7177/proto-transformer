#pragma once

#include<memory>

namespace TricksAndThings { namespace LockFree
{

template<class P>
struct DefaultAllocatingStorage
{
    template<class... Args>
    static P *make(Args &&... args)
    {
        return new P(std::forward<Args>(args)...);
    }

    static void utilize(P *p)
    {
        std::unique_ptr<P> dropIt(p);
    }
};

} }
