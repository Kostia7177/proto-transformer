#pragma once

#include "Tools.hpp"
#include <cstddef>

namespace ProtoTransformer
{

namespace Serializer
{

enum { fit2Calculated = 0 };

template<typename T, int k>
struct FieldDesc
{
    typedef T Type;
    enum { key = k };
};

template<size_t bufferSize, typename... FieldDescs>
class IntegralsOnly
{
    template<typename T, size_t offs>
    struct Bind
    {   // common and a simplest case - intergral type binder
        typedef T Type2Set;
        typedef T Type2Get;
        typedef T Type2Export;
        const static size_t offset = offs;

        static void set(
            Type2Set value,
            unsigned char *addr)
        {
            *(Type2Set *)addr = ntoh(value, Int2Type<sizeof(Type2Set)>());
        }

        static Type2Get get(const unsigned char *addr)
        {
            return hton(*(Type2Get *)addr, Int2Type<sizeof(Type2Get)>());
        }

        static void export2(const unsigned char *addr, Type2Export &value){ value = get(addr); }
    };

    template<typename T, size_t length, size_t offs>
    struct Bind<T[length], offs>
    {   // array binder
        typedef T Type2Set[length];
        typedef T *Type2Get;
        typedef Type2Set Type2Export;
        static const size_t offset = offs;

        static void set(
            const Type2Set &value,
            unsigned char *addr)
        {
            for (size_t idx = 0; idx < length; ++ idx)
            {
                ((Type2Get)addr)[idx] = ntoh(value[idx], Int2Type<sizeof(T)>());
            }
        }

        static Type2Get get(unsigned char *)
        {
            struct NotSame {};
            static_assert(std::is_same<T, NotSame>::value,
                          "\nNo 'get' method allowed for array types; use an\n"
                          "\t'export2' method to retrieve a copy of it's data;\n");
            return 0;
        }

        static void export2(
            const unsigned char *addr,
            Type2Export &buffer)
        {
            for (size_t idx = 0; idx < length; ++ idx)
            {
                buffer[idx] = hton(((Type2Get)addr)[idx], Int2Type<sizeof(T)>());
            }
        }
    };

    template<size_t fieldSize, size_t offs>
    struct Bind<char[fieldSize], offs>
    {   // c-string binder
        typedef const char *Type2Set;
        typedef const char *Type2Get;
        typedef char Type2Export[fieldSize];
        static const size_t offset = offs;

        static void set(
            Type2Set value,
            unsigned char *addr)
        {
            size_t length = strlen(value); 
            if (length >= fieldSize) { length = fieldSize - 1; }
            memcpy(addr, value, length);
            addr[length] = 0;
        }

        static Type2Get get(const unsigned char *addr){ return (Type2Get)addr; }

        static void export2(const unsigned char *addr, Type2Export &buf){ memcpy(buf, addr, fieldSize); }
    };

    template<int,size_t,typename...>struct Core;

    template<int idx, size_t offset, typename Head, typename... Tail>
    struct Core<idx, offset, Head, Tail...>
    {
        typedef typename Head::Type Type;
        typedef Core<idx + 1, offset + sizeof(Type), Tail...> FollowingFields;

        typedef typename FollowingFields::Buf Buf;

        template<int pos, bool = pos == Head::key> struct Navigate2;

        template<int pos>
        struct Navigate2<pos, false>
        {
            static_assert(pos < sizeof...(FieldDescs),
                          "\n\n\tPosition 'pos' is  out of range! \n");

            typedef typename FollowingFields
                        ::Core
                        ::template Navigate2<pos>
                        ::Field Field;  // sees to -----------------+
        };  //                                                      |
        //                                                          |
        template<int pos>   //                                      |
        struct Navigate2<pos, true> //                              |
        {   // border case                                          |
            typedef Bind<Type, bufferSize != fit2Calculated ?   //  |
                                bufferSize  //                      |
                                : offset> Field;  // <--------------+
        };
    };

    typedef Core<0, 0, FieldDescs...> CoreImpl;

    template<int idx, size_t sizeOfBuf>
    struct Core<idx, sizeOfBuf>
    {
        class Buf
        {
            unsigned char data[sizeOfBuf];

            public:

            template<int pos>
            void set(const typename CoreImpl::template Navigate2<pos>::Field::Type2Set &value)
            {
                typedef typename CoreImpl::template Navigate2<pos>::Field Field;
                Field::set(value, data + Field::offset);
            }

            template<int pos>
            typename CoreImpl::template Navigate2<pos>::Field::Type2Get get() const
            {
                typedef typename CoreImpl
                            ::template Navigate2<pos>
                            ::Field Field;
                return Field::get(data + Field::offset);
            }

            template<int pos>
            void exportField(typename CoreImpl::template Navigate2<pos>::Field::Type2Export &outBuffer) const
            {
                typedef typename CoreImpl::template Navigate2<pos>::Field Field;
                Field::export2(data + Field::offset, outBuffer);
            }
        };
    };

    template<typename T, int size>
    static T ntoh(T t, const Int2Type<size> &)
    {
        return t;
    }
    template<typename T, int size>
    static T hton(T t, const Int2Type<size> &)
    {
        return t;
    }
    template<typename T>static T ntoh(T t, const Int2Type<16> &){ return ntohs(t); }
    template<typename T>static T hton(T t, const Int2Type<16> &){ return htons(t); }
    template<typename T>static T ntoh(T t, const Int2Type<32> &){ return ntohl(t); }
    template<typename T>static T hton(T t, const Int2Type<32> &){ return htonl(t); }

    public:

    typedef typename CoreImpl::Buf Buffer;
};
}
}
