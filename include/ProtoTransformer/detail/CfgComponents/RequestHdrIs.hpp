#pragma once

#include "../Tools.hpp"
#include<cstddef>

namespace ProtoTransformer
{

template<class T, class Base = NullType>
struct RequestHdrIs : virtual Base
{
    typedef T RequestHdr;

    static_assert(IsTransferable<typename T::Itself>::value == true,
                  "Request or answer data header must be either arithmetic, or pod. "
                  "Or NullType.");

    enum { proto = 1 };
};

template<class Base>
struct RequestHdrIs<NullType, Base> : virtual Base
{
    struct RequestHdr
    {
        typedef NullType Itself;

        static uint32_t getSize(Itself)             { return 0; }
        static void setSize2(uint32_t, Itself &)    {}
    };

    enum { proto = 1 };
};

}
