#pragma once

#include "UsefulDefs.hpp"

namespace TricksAndThings { namespace LockFree { namespace detail
{

template<class Mode = CasMode<mem::acquire, mem::release>, typename T, class F>
bool casIf(
    std::atomic<T> &value,
    F f)
{
    T valueItself, valueBefore(value.load(Mode::loading));
    do
    {
        valueItself = valueBefore;
        if (f(valueItself) == false) { return false; }
    }
    while (!value.compare_exchange_strong(valueBefore, valueItself,
                                          Mode::storing,
                                          mem::relaxed));
    return true;
}

} } }
