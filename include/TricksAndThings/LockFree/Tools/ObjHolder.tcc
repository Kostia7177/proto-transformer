#include "../detail/casIf.hpp"

namespace TricksAndThings
{
namespace LockFree
{

template<class T, class Storage>
ObjHolder<T, Storage>::Ptr::~Ptr()
{
    if (ptr) { ptr->unRef(); }
}

template<class T, class Storage>
typename ObjHolder<T, Storage>::Ptr &ObjHolder<T, Storage>::Ptr::operator=(Ptr &&arg)
{
    swap(std::move(arg));
    return *this;
}

template<class T, class Storage>
void ObjHolder<T, Storage>::makeIfNull(
    Itself &p,
    T *&newPtr)
{
    if (newPtr)
    {
        Storage::utilize(newPtr);
        newPtr = 0;
    }
    if (!p.ptr)
    {
        newPtr = Storage::make();
        p.ptr = newPtr;
    }
}

template<class T, class Storage>
ObjHolder<T, Storage>::~ObjHolder()
{
    if (T *ptr = itself.load(mem::acquire).ptr)
    {
        ptr->unRef();
    }
}

template<class T, class Storage>
typename ObjHolder<T, Storage>::Ptr ObjHolder<T, Storage>::getPtrStrict()
{
    T *newPtr = 0;
    detail::casIf<Free>(itself, [&](Itself &p)
                                {
                                    makeIfNull(p, newPtr);
                                    ++ p.inProgress;
                                    return true;
                                });
    return *this;
}

template<class T, class Storage>
T *ObjHolder<T, Storage>::getSubj()
{
    T *newPtr = 0;
    detail::casIf<Free>(itself, [&](Itself &p)
                                { return makeIfNull(p, newPtr), true; });
    return itself.load(mem::relaxed).ptr;
}

template<class T, class Storage>
typename ObjHolder<T, Storage>::Ptr ObjHolder<T, Storage>::get()
{
    detail::casIf<AsConsumer>(itself, [](Itself &p)
                                      {
                                        if (!p.ptr) { return false; }
                                        ++ p.inProgress;
                                        return true;
                                      });
    return *this;
}

template<class T, class Storage>
void ObjHolder<T, Storage>::setIfNull(T *arg)
{
    detail::casIf<Free>(itself, [=](Itself &p)
                                {
                                    if (p.ptr) { return false; }
                                    p.ptr = arg;
                                    return true;
                                });
}

template<class T, class Storage>
T *ObjHolder<T, Storage>::refere2()
{
    T *subj = itself.load(mem::relaxed).ptr;
    if (subj)
    {
        subj->refOn();
        detail::casIf<AsProducer>(itself, [](Itself &p)
                                          {
                                            -- p.inProgress;
                                            return true;
                                          });
    }
    return subj;
}

template<class T, class Storage>
template<class C>
bool ObjHolder<T, Storage>::swapIf(
    Ptr &&arg,
    C condition)
{
    T *orig;
    if (!detail::casIf<>(itself, [&](Itself &p)
                                 {
                                    if (p.inProgress
                                        || !condition(p.ptr, arg.ptr))
                                    {
                                        return false;
                                    }
                                    orig = p.ptr;
                                    p.ptr = arg.ptr;
                                    return true;
                                 }))
    {
        return false;
    }
    arg.ptr = orig;
    return true;
}
}
}
