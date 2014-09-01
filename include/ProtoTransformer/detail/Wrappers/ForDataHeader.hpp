#pragma once

#include "../Tools.hpp"

namespace ProtoTransformer
{
namespace Wrappers
{

template<class T>
struct ForDataHeader
{
    typedef T Type;
    static_assert(IsTransferable<typename Type::Itself>::value == true,
                  "Request or answer data header must be either arithmetic, or pod. "
                  "Or NullType.");
};

template<>
struct ForDataHeader<NullType>
{
    struct Type
    {
        typedef NullType Itself;

        static uint32_t getSize(Itself)             { return 0; }
        static void setSize2(uint32_t, Itself &)    {}
    };
};

}
}
