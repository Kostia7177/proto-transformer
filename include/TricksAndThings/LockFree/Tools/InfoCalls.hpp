#pragma once

#include "../detail/UsefulDefs.hpp"
#include <type_traits>

namespace TricksAndThings
{
namespace LockFree
{
namespace Queues
{
namespace Components
{

template<typename>
class WithInfoCalls
{
    SizeAtomic value;
    protected:
    void incrSize() { ++ value; }
    void decrSize() { -- value; }
    WithInfoCalls() : value(0){}
    ~WithInfoCalls(){}
    public:
    bool empty()    { return !size(); }
    size_t size()   { return value.load(); }
};

template<typename T>
class NoInfoCalls
{
    T x;
    size_t fail(T)
    {
        struct Inaccessable{};
        static_assert(std::is_same<T, Inaccessable>::value,
                      "\n\n\tThe 'size()' and 'empty()' methos are not implemented by default."
                      "\n\tTo call them use the 'WithInfoCalls' configuration trait;\n");
        return 0;
    }
    protected:
    void incrSize() {}
    void decrSize() {}
    public:
    bool empty()    { return !size(); }
    size_t size()   { return fail(x); }
};

}
}
}
}
