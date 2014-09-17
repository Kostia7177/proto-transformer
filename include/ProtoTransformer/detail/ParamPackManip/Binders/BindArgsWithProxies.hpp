#pragma once

#include "BindArgs.hpp"
#include "../../Wrappers/ForDataHeader.hpp"
#include "../../JustSize.hpp"

namespace ProtoTransformer
{

template<int idx, typename T>
struct BindArgs<idx, Wrappers::ForDataHeader<T &>>
    : BindArgs<idx, typename T::Itself &>
{
};

template<int idx, typename T>
struct BindArgs<idx, Wrappers::ForDataHeader<T *>>
    : BindArgs<idx, typename T::Itself *>
{
};

template<int idx, typename T>
struct Proxy4JustSize
    : BindArgs<idx, T>
{
    JustSize::Itself pointee;
    Proxy4JustSize(){ this->value = &pointee; }
    enum { assignable = false };
};

template<int idx>
struct BindArgs<idx, Wrappers::ForDataHeader<JustSize &>>
    : Proxy4JustSize<idx, JustSize::Itself &>
{
};

template<int idx>
struct BindArgs<idx, Wrappers::ForDataHeader<JustSize *>>
    : Proxy4JustSize<idx, JustSize::Itself *>
{
};

}
