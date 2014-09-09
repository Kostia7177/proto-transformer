#pragma once

#include <type_traits>
#include <thread>
#include <boost/asio.hpp>

namespace ProtoTransformer
{
namespace Asio = boost::asio;
namespace Ip = Asio::ip;
typedef Ip::tcp::socket Socket;
namespace Sys = boost::system;
}

template<int arg> using Int2Type = std::integral_constant<int, arg>;
struct NullType {};

// a pair of types with guarantee-different sizes.
// used within any sfinae-based detectors.
typedef char One;
struct Two { One two[2]; };

template<typename T, int>
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
    template<class Base> class Derived : public Base { char c; };
    static const bool value = sizeof(Derived<T>) > sizeof(Derived<NullType>);
};

enum { hardwareConcurrency = 0 };
inline unsigned int getNumOfThreads(unsigned int numOf)
{
    return numOf == hardwareConcurrency ? std::thread::hardware_concurrency() : numOf;
}
