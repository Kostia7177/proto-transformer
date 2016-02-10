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
        template<class W> void startSession(std::shared_ptr<S> session, W &workflow)
        { session->run(workflow); }
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
