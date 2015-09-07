#pragma once

#include"../../TricksAndThings/Tools/OneTwo.hpp"
#include"../../TricksAndThings/Tools/Int2Type.hpp"

namespace ProtoTransformer {

template<class T> One withBeforeStopActions(const typename T::BeforeServerStop *);
template<typename> Two withBeforeStopActions(...);

template<class H> void beforeStopSw(const Int2Type<true> &, const H &, int sigNum)
{ typename H::BeforeServerStop()(sigNum); }

template<class H> void beforeStopSw(const Int2Type<false> &, const H &, int)
{}

}
