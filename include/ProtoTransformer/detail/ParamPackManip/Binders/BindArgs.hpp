#pragma once

#include "Bases.hpp"

namespace ProtoTransformer
{

template<typename T>
struct CleanupQualifiers
{
    typedef typename std::remove_pointer<
            typename std::remove_reference<T>::type>::type NoRefNoPtr;
    typedef typename std::remove_const<NoRefNoPtr>::type Core;
};

template<int idx, typename T, int uniquizer>
class BindArgs
    : public BinderData<typename CleanupQualifiers<T>::NoRefNoPtr *>,
      public BinderPos<idx + 1>
{
    typedef Int2Type<true> Pointer;
    typedef Int2Type<true> Reference;
    typedef Int2Type<false> Other;
    template<typename R> static R *getPtrSw(const Other &, const Reference &, R &r){ return &r; }
    template<typename R> static const R *getPtrSw(const Other &, const Reference &, const R &r){ return &r; }
    template<typename P> static P getPtrSw(const Pointer &, const Other &, P p){ return p; }

    public:

    enum
    {
        // in general, all things are assignable, with exception of null-type and
        // all it's references or pointers;
        assignable = !std::is_same<NullType, typename CleanupQualifiers<T>::Core>::value,
        pointerRequired = std::is_pointer<T>::value,
        referenceRequired = std::is_reference<T>::value,
        constRequired = std::is_const<T>::value
    };

    template<typename Arg>
    void populate(Arg &&arg)
    {
        static_assert(!pointerRequired
                      || pointerRequired && std::is_pointer<typename std::remove_reference<Arg>::type>::value,
                      "\n\n\tPointer needed while object or reference passed as an argunemt! "
                      "\n\tSee the following message for details. \n");
        static_assert(!referenceRequired
                      || referenceRequired && !std::is_pointer<typename std::remove_reference<Arg>::type>::value,
                      "\n\n\tReference needed while pointer passed as an argument! "
                      "\n\tSee the following message for details. \n");
        this->value = getPtrSw(Int2Type<pointerRequired>(),
                               Int2Type<std::is_reference<T>::value>(),
                               arg);
    }

    void populate(){}
};

}
