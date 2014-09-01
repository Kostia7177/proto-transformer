#pragma once
#include <type_traits>
#include "Params2Hierarchy.hpp"
#include "Hierarchy2Params.hpp"
#include "Binders/BindNotNullsOnly.hpp"

namespace ProtoTransformer
{

template<class F, typename... Params>
int filteringAdapter(F f, Params &... params)
{   // -- aim:
    //
    // suppose there is a fixed-size set of variable types:
    //
    //  t1       t2       t3       t4       t5
    //
    // that are defined inside any traits structure. any of theese types
    // can be instantiated by an 'empty' NullType, and any others - by
    // some meaningfull Ti:
    //
    //  T1       NullType T3       T4       NullType    -- (ex 1)
    //  NullType NullType NullType NullType T5          -- (ex 2)
    //  T1       T2       NullType T4       NullType    -- (ex 3)
    //  NullType T2       NullType NullType T5          -- (ex 4)
    //
    // suppose now, that we want to pass the objects of theese types to any client's
    // function, which knows nothing about our 'NullType' (and, worst of all, don't
    // want to know).
    // for our four cases of theese client's functions are:
    //
    //  int f1(T1, T3, T4);     // ex 1
    //  int f2(T5);             // ex 2
    //  int f3(T1, T2, T4);     // ex 3
    //  int f4(T2, T5);         // ex 4
    //
    // so, all we've got to do is to filter out all the NullType-objects and to pass
    // the filtered set to a client's code.
    //
    // -- implementation:
    //
    // 'Params...' contains the input (dirty) set of types;
    typedef Params2Hierarchy<BindNotNullsOnly, Params...> Hierarchy;
    Hierarchy h(params...);  // simply creating a 'filtered hierarchy'...
    // ...and simply building a new, filtered, variadic. And then simply call a payload,
    return Hierarchy2Params<Hierarchy>::call(f, h);
    // ...and nothing more.
}
}
