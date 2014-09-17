#pragma once

#include "../../Tools.hpp"
#include "Bases.hpp"

namespace ProtoTransformer
{

template<int idx, typename T>
struct BindNotNullsOnly
    : BinderData<T &>,
      BinderPos<idx + 1>
{   // common case; puts a reference to 'T' as
    // a payload field;
    BindNotNullsOnly(T &arg) : BinderData<T &>(arg){}
};

template<int idx, typename Pointee>
struct BindNotNullsOnly<idx, Pointee *>
    : BinderData<Pointee *>,
      BinderPos<idx + 1>
{   // pointers must be represented
    // as is, not as referenses;
    BindNotNullsOnly(Pointee *arg) : BinderData<Pointee *>(arg) {}
};

template<int idx>
struct BindNotNullsOnly<idx, NullType>
    : BinderPos<idx>
{   // hiding the meanless 'NullType'
    BindNotNullsOnly(NullType) {}
};

template<int idx>
struct BindNotNullsOnly<idx, const NullType>
    : BinderPos<idx>
{
    BindNotNullsOnly(NullType) {}
};

template<int idx>
struct BindNotNullsOnly<idx, NullType *>
    : BinderPos<idx>
{
    BindNotNullsOnly(NullType *) {}
};

}
