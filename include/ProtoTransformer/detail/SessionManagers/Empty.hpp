#pragma once

#include "Base.hpp"
#include <memory>

namespace ProtoTransformer
{

struct EmptyManager
{
    template<class S>
    struct Itself
    {
        template<class F> void startSession(std::shared_ptr<S> session, F payload){ session->run(payload); }
    };

    template<class S>
    struct Reference
    {
        bool sessionWasRemoved(){ return false; }
    };

    template<class S>
    struct ExitDetector : ExitDetectorBase<S>
    {
        ExitDetector(typename std::shared_ptr<S> arg) : ExitDetectorBase<S>(arg){}
    };
};

}
