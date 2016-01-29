#pragma once

#include <memory>

namespace ProtoTransformer
{

template<class S>
class ExitDetectorBase
{   // смысл этой базы - подчеркнуть семантику детектора выхода,
    // иного смысла (архитектурного или ещё какого) она не имеет.
    // каждый детектор выхода должен содержать хранитель сессии,
    // и конечно, можно было бы их разместить и непосредственно
    // в теле наследника.
    std::shared_ptr<S> sessionKeeper;
    protected:
    ExitDetectorBase(std::shared_ptr<S> arg) : sessionKeeper(arg){}
    virtual ~ExitDetectorBase(){}
    public:
    S &getSession(){ return *sessionKeeper; }
};

}
