#pragma once

namespace ProtoTransformer
{

template<typename... Params> struct Params2TypesHierarchy;

template<typename Head, typename... Tail>
struct Params2TypesHierarchy<Head, Tail...>
{
    struct Type : Head, Params2TypesHierarchy<Tail...>::Type{};
};

template<typename Last>
struct Params2TypesHierarchy<Last>
{
    typedef Last Type;
};

}
