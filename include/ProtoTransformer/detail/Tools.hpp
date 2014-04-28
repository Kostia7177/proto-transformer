#pragma once

#include <type_traits>
#include <thread>

template<int arg> using Int2Type = std::integral_constant<int, arg>;
struct NullType {};

template<class T, int meaningful>
struct ReplaceWithNullIf2nd
{
    typedef T Type;
    Type &value;
    ReplaceWithNullIf2nd(T &arg) : value(arg){}
};

template<class T>
struct ReplaceWithNullIf2nd<T, true>
{
    typedef NullType Type;
    NullType value;
    ReplaceWithNullIf2nd(T &){}
};
template<typename... Params> struct Params2Hierarchy;
template<typename Head, typename... Tail>
struct Params2Hierarchy<Head, Tail...>
{
    struct Type : Head, Params2Hierarchy<Tail...>::Type{};
};

template<typename Last>
struct Params2Hierarchy<Last>
{
    typedef Last Type;
};

template<typename P>
struct OneParamChecker
{
    static_assert(P::proto == 0,
                  "Proto component cannot be used outside of the proto! ");
};

template<typename... > struct NonProtoParamsChecker;

template<>
struct NonProtoParamsChecker<>
{
    NullType simplyOk;
};

template<typename Last>
struct NonProtoParamsChecker<Last>
{
    OneParamChecker<Last> lastOk;
};

template<typename Head, typename... Tail>
struct NonProtoParamsChecker<Head, Tail...>
{
    OneParamChecker<Head> firstOk;
    NonProtoParamsChecker<Tail...> othersOk;
};

template<class T,
         bool = std::is_arithmetic<T>::value,
         bool = std::is_pod<T>::value>
struct IsTransferable
{
};

template<class T, bool i>
struct IsTransferable<T,
                      true,
                      i>
{   // something simple (char, kind of int, float) is almost transferable;
    static const bool value = true;
};

template<class T>
struct IsTransferable<T,
                      false,
                      true>
{
    template<class Base> class ParamDerived : public Base { char c; };
    static const bool value = sizeof(ParamDerived<T>) > sizeof(ParamDerived<NullType>);
};

template<class T, bool toggle> struct EmptyOrNot { typedef T type; };
template<class T> struct EmptyOrNot<T, false> { typedef NullType type; };

enum { hardwareConcurrency = 0 };
unsigned int getNumOfThreads(unsigned int numOf)
{
    return numOf == hardwareConcurrency ? std::thread::hardware_concurrency() : numOf;
}
