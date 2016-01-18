#pragma once

#include "../../TricksAndThings/SignatureManip/detail/Binders/BindArgs.hpp"
#include "JustSize.hpp"
namespace ProtoTransformer { namespace detail
{
template<typename> struct DataHeaderWrapper {};
} }

namespace TricksAndThings
{
namespace Ptd = ProtoTransformer::detail;

template<int idx, typename T>
struct BindArgs<idx, Ptd::DataHeaderWrapper<T &>>
    : BindArgs<idx, typename T::Itself &>
{
};

template<int idx, typename T>
struct BindArgs<idx, Ptd::DataHeaderWrapper<T *>>
    : BindArgs<idx, typename T::Itself *>
{
};

typedef ProtoTransformer::JustSize JustSize;

template<int idx, typename T>
struct Proxy4JustSize
    : BindArgs<idx, T>
{
    JustSize::Itself pointee;
    Proxy4JustSize(){ this->value = &pointee; }
    enum { assignable = false };
};

template<int idx>
struct BindArgs<idx, Ptd::DataHeaderWrapper<JustSize &>>
    : Proxy4JustSize<idx, JustSize::Itself &>
{
};

template<int idx>
struct BindArgs<idx, Ptd::DataHeaderWrapper<JustSize *>>
    : Proxy4JustSize<idx, JustSize::Itself *>
{
};

}
