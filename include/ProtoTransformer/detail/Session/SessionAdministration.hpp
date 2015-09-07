#pragma once

#include "../Tools.hpp"

namespace ProtoTransformer {

template<class Cfg, class S>
struct SessionAdministration
{
    // a thing that controls the reading,
    // if no the request size is known...
    typedef typename Cfg::
                     ReadingManager::
                     template Itself<typename Cfg::RequestCompletion,
                                     typename Cfg::RequestDataRepr> RequestReadingManager;

    RequestReadingManager readingManager;
    // ...but it's completion can be recognized
    // while reading the request body - the following
    // is a user-defined code, that it provides;
    typedef typename RequestReadingManager::Completion RequestCompletion;

    typedef typename Cfg::SessionManager::template Reference<S> ExitManager;
    ExitManager exitManager;
    typename Cfg::TaskManager taskManager;

    template<class B>
    SessionAdministration(B &buffer)
        : readingManager(buffer),
          taskManager(getNumOfThreads(Cfg::numOfRequestsPerSession)){}
};

}
