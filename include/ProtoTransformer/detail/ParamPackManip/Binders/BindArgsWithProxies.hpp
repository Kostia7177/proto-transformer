#pragma once

#include "BindArgs.hpp"
#include "../../Wrappers/ForDataHeader.hpp"
#include "../../JustSize.hpp"

namespace ProtoTransformer
{

template<int idx, typename T, int uniquizer>
struct BindArgs<idx, Wrappers::ForDataHeader<T &>, uniquizer>
    : BindArgs<idx, typename T::Itself &, uniquizer>
{
};

template<int idx, typename T, int uniquizer>
struct BindArgs<idx, Wrappers::ForDataHeader<T *>, uniquizer>
    : BindArgs<idx, typename T::Itself *, uniquizer>
{
};

template<int idx, typename T, int uniquizer>
struct Proxy4JustSize
    : BindArgs<idx, T, uniquizer>
{
    JustSize::Itself pointee;
    Proxy4JustSize(){ this->value = &pointee; }
    enum { assignable = false };
};

template<int idx, int uniquizer>
struct BindArgs<idx, Wrappers::ForDataHeader<JustSize &>, uniquizer>
    : Proxy4JustSize<idx, JustSize::Itself &, uniquizer>
{
};

template<int idx, int uniquizer>
struct BindArgs<idx, Wrappers::ForDataHeader<JustSize *>, uniquizer>
    : Proxy4JustSize<idx, JustSize::Itself *, uniquizer>
{
};

}
