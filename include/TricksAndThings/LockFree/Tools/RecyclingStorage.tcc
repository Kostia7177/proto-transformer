#include "../detail/casIf.hpp"
#include<memory>

namespace TricksAndThings { namespace LockFree
{
template<class P>
void RecyclingStorage<P>::cleanup()
{
    do
    {
        RawBuf *toBeDropped;
        if (detail::casIf<>(data.itself, [&](RevisedPtr &p)
                                {
                                    if (!p.ptr){ return false; }
                                    toBeDropped = p.ptr;
                                    p.ptr = *p.ptr->nextPtr();
                                    ++ p.rev;
                                    return true;
                                }))
        {
            std::unique_ptr<RawBuf> utilizer(toBeDropped);
        }
        else { return; }
    }
    while (1);
}
template<class P>
template<typename... Args>
P *RecyclingStorage<P>::make(Args &&... args)
{
    RawBuf *ret;
    detail::casIf<Free>(data.itself, [&](RevisedPtr &p)
                   {
                    RawBuf *&ptrRef = p.ptr;
                    if (!ptrRef)
                    {
                        ret = new RawBuf(0);
                        return false;
                    }
                    ret = ptrRef;
                    ptrRef = *ret->nextPtr();
                    ++ p.rev;
                    return true;
                   });
    new(ret) P(std::forward<Args>(args)...);
    return reinterpret_cast<P *>(ret);
}
template<class P>
void RecyclingStorage<P>::utilize(P *arg)
{
    arg->~P();
    detail::casIf<Free>(data.itself, [=](RevisedPtr &p)
                   {
                    new(arg) RawBuf(p.ptr);
                    p.ptr = reinterpret_cast<RawBuf *>(arg);
                    ++ p.rev;
                    return true;
                   });
}
} }
