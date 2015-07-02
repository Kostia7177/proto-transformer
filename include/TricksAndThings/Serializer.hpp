#pragma once
/*
    Integral types serializer;

    Copyright (C) 2014  Konstantin U. Zozoulia

    candid.71 -at- mail -dot- ru

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//
// about:
//
// a simple tool to serialize (and deserialize) a pod-like-structures data;
// does it
//  - easy and clear;
//  - type-secured;
//  - fast;
// size of an output product is known at a compile time, what makes it usable
// when application reads any fixed-size header of a following message, or
// when you want to pass any vector<serialized data> ;
//
// appliable to integral types and their arrays. array of char interpreted
// as a c-string
//
// usage:
//
// alias the field descriptor:
//
//   template<typename F, int pos> using SerFD = Serializer::FieldDesc<F, pos>;
//
// define field names:
//
//   enum { firstField, otherField, anyTextField, someArrayField };
//
// define a serializer:
//
//   typedef Serializer::IntegralsOnly
//           <
//              Serializer::fit2Calculated, // fit the output buffer to the summary
//                                          // size of it's fields (or specify the
//                                          // reserved buffer size);
//              // fields:
//              SerFD<uint32_t,         firstField>,
//              SerFD<char,             otherField>,
//              SerFD<char[4242],       anyTextField>,
//              SerFD<int32_t[5],       someArrayField>
//           >
//           ::Buffer MySerializer;
//
// use it in code:
//
//   MySerializer buffer;
//
//   buffer.set<firstField>(42);
//   uint32_t u = buffer.get<firstField>();
//
//   int32_t i[5] = { 11, 12, 13, 14, 15 };
//   buffer.set<someArrayField>(i);
//   buffer.get<someArrayField>()[3] = 33;
//   int32_t i1[5];
//   buffer.export2<someArrayField>(i1);
//
//   buffer.set<anyTextField>("hello, serializer!");
//   buffer.get<anyTextField>();
//   const char *text = buffer.get<anyTExtField>();
//

#include "Tools.hpp"
#include <cstddef>

namespace ProtoTransformer
{

namespace Serializer
{

template<typename T, int k>
struct FieldDesc
{   // describer of the fields that are to be serialized;
    // wraps field's type and a named numeric value as such
    // a "virtual structure"'s field;
    // any sequence of theese field descriptions as a
    // parameter pack of serializer's template represents
    // a 'virtual structure' that is to be serialized;
    typedef T Type;     // type of the field;
    enum { key = k };   // it's kinda name;
};

enum { fit2Calculated = 0 };
enum { valueAsIs = true };

template<typename T>
constexpr size_t muxBySize(bool raw)
{ return raw ? 0 : sizeof(T); }

template<size_t bufferSize, typename... FieldDescs>
class IntegralsOnly
{   // integral types (and their arrays) serializer;
    //
    // 'Bind' structures contain the following:
    //   - types of input and output data;
    //   - offset of the field's data at the
    //     "production buffer";
    //   - accessing methods;
    template<typename T, size_t offs>
    struct Bind
    {   // common and a simplest case - intergral type binder
        typedef T Type2Set;
        typedef T Type2Get;
        typedef T Type2Export;
        const static size_t offset = offs;

        template<bool raw>
        static void set(Type2Set value, unsigned char *addr)
        { *(Type2Set *)addr = hton(value, Int2Type<muxBySize<Type2Set>(raw)>()); }

        template<bool raw>
        static Type2Get get(const unsigned char *addr)
        { return ntoh(*(Type2Get *)addr, Int2Type<muxBySize<Type2Get>(raw)>()); }

        template<bool raw>
        static void export2(const unsigned char *addr, Type2Export &value)
        { value = get<raw>(addr); }
    };

    template<typename T, size_t length, size_t offs>
    struct Bind<T[length], offs>
    {   // array binder
        class Type2Get
        {   // proxy class, invisible for a client's code;
            // implements read and write access to array's
            // elements by operator[];
            T *data;
            size_t idx;

            public:

            Type2Get(T *addr) : data(addr), idx(0){}

            Type2Get &operator[](size_t i)  { return idx = i, *this; }
            // access for writing;
            T operator=(const T &value)
            { return data[idx] = hton(value, Int2Type<sizeof(T)>()), value; }
            // access for reading;
            operator T()        { return ntoh(data[idx], Int2Type<sizeof(T)>()); }

            static const size_t size = length;
        };

        typedef T Type2Set[length];
        typedef Type2Set Type2Export;
        static const size_t offset = offs;

        template<bool raw>
        static void set(
            const Type2Set &value,
            unsigned char *addr)
        {
            std::transform(value, value + length,
                           (T *)addr,
                           [=] (T arg)  { return hton(arg, Int2Type<muxBySize<T>(raw)>()); });
        }

        // meaningful only for an array-like subscription and a size request
        // see 'Type2Get' proxy class comments;
        template<bool raw>
        static Type2Get get(const unsigned char *addr)
        {
            static_assert(raw == false,
                          "\n\n\tSorry, no raw access to array's elements implemented yet - use\n"
                          "\t'set<fieldName, valueAsIs>' or 'export2<fieldName, valueAsIs>'\n"
                          "\tto set or get array's data as is;\n");
            return Type2Get((T *)addr);
        }

        template<bool raw>
        static void export2(
            const unsigned char *addr,
            Type2Export &buffer)
        {
            std::transform((const T *)addr, (const T *)addr + length,
                           buffer,
                           [=](T arg)   { return ntoh(arg, Int2Type<muxBySize<T>(raw)>()); });
        }
    };

    template<size_t fieldSize, size_t offs>
    struct Bind<char[fieldSize], offs>
    {   // c-string binder
        typedef const char *Type2Set;
        typedef const char *Type2Get;
        typedef char Type2Export[fieldSize];
        static const size_t offset = offs;

        template<bool>
        static void set(
            Type2Set value,
            unsigned char *addr)
        {
            size_t length = strlen(value);
            if (length >= fieldSize) { length = fieldSize - 1; }
            memcpy(addr, value, length);
            addr[length] = 0;
        }

        template<bool>
        static Type2Get get(const unsigned char *addr)
        { return (Type2Get)addr; }

        template<bool>
        static void export2(const unsigned char *addr, Type2Export &buf)
        { memcpy(buf, addr, fieldSize); }
    };

    // metaprogram that maps fields of a 'virtual structure' to
    // the 'production buffer'
    template
        <
            size_t,     // - integrated size of fields that are passed,
                        // at the bottom of recursion means calculated 
                        // size of the buffer, or (otherwise, not at the
                        // bottom) current field's offset;
            typename... // - rest of fields;
        >struct Core;
    //
    template<size_t offset, typename Head, typename... Tail>
    struct Core<offset, Head, Tail...>
    {
        typedef typename Head::Type Type;
        typedef Core<offset + sizeof(Type), Tail...> FollowingFields;

        typedef typename FollowingFields::Buf Buf;  // actually sees to ----+
                                                //                          |
        // a metaprogram that finds required field by it's key;             |
        template<int key, bool = key == Head::key> struct Navigate2;    //  |
        //                                                                  |
        template<int key>   //                                              |
        struct Navigate2<key, false>    //                                  |
        {   // not this field - go forward;                                 |
            //                                                              |
            // first, check whether any place to go;                        |
            // sfinae-based detector:                                       |
            //  - yes, not at the last field...;                            |
            template<class C>   //                                          |
            static One notAtLastField(  //                                  |
                typename C::template Navigate2<key> *); //                  |
            //  - ...and no, realy at the last field;                       |
            template<typename> static Two notAtLastField(...);  //          |
            //                                                              |
            typedef typename FollowingFields::Core FollowingCore;   //      |
            // compile-time error message humanizer;                        |
            static_assert(sizeof(notAtLastField<FollowingCore>(0))  //      |
                          == sizeof(One),  //                               |
                          "\n\n\tField with this key (see " //              |
                          "the line number\n\tat the previous"  //          |
                          " error message) not found! \n"); //              |
                          //                                                |
            typedef typename FollowingCore   //                             |
                        ::template Navigate2<key>  //                       |
                        ::Field Field;  // in fact, sees to --------+       |
        };  //                                                      |       |
        //                                                          |       |
        template<int key>   //                                      |       |
        struct Navigate2<key, true> //                              |       |
        {   // border case - required field is found;               |       |
            typedef Bind<Type, offset> Field;  // <-----------------+       |
        };  //                                                              |
    };  //                                                                  |
    //                                                                      |
    typedef Core<0, FieldDescs...> CoreImpl;    // serializer's core itself |
    //                                                                      |
    template<size_t sizeOfBuf>  //                                          |
    struct Core<sizeOfBuf>  //                                              |
    {   // border case - parameter pack exhausted, size of the              |
        // 'production buffer' and the offsets of all fields calculated     |
        //                                                                  |
        class Buf   // <----------------------------------------------------+
        {   // serializer's working body itself;
            unsigned char data[bufferSize != fit2Calculated ?
                                bufferSize
                                : sizeOfBuf];  // serialized data - 'production buffer';

            public: // (!!!) public interface of the serializer is here;

            template<int pos, bool raw = false>
            void set(const typename CoreImpl::template Navigate2<pos>::Field::Type2Set &value)
            {
                typedef typename CoreImpl::template Navigate2<pos>::Field Field;
                Field::template set<raw>(value, data + Field::offset);
            }

            template<int pos, bool raw = false>
            typename CoreImpl::template Navigate2<pos>::Field::Type2Get get() const
            {
                typedef typename CoreImpl
                            ::template Navigate2<pos>
                            ::Field Field;
                return Field::template get<raw>(data + Field::offset);
            }

            template<int pos, bool raw = false>
            void export2(typename CoreImpl::template Navigate2<pos>::Field::Type2Export &outBuffer) const
            {
                typedef typename CoreImpl::template Navigate2<pos>::Field Field;
                Field::template export2<raw>(data + Field::offset, outBuffer);
            }
        };
    };

    template<typename T, int size>
    static T ntoh(T t, const Int2Type<size> &)
    {
        static_assert(!size             // 'value as is' requested,
                      || (size == 1),   // or it is a single char;
                      // all other cases are illegal;
                      "\n\n\tNo network to host conversion is known for a\n"
                      "\tlong type - use 'get<fieldName, valueAsIs>'\n"
                      "\tto allow 'raw' (as is) serialization;\n");
        return t;
    }
    template<typename T, int size>
    static T hton(T t, const Int2Type<size> &)
    {
        static_assert(!size             // for comments see 'ntoh';
                      || (size == 1),
                      "\n\n\tNo host to network conversion is known for a\n"
                      "\tlong type - use 'set<fieldName, valueAsIs>'\n"
                      "\tto allow 'raw' (as is) serialization;\n");
        return t;
    }
    template<typename T>static T ntoh(T t, const Int2Type<2> &){ return ntohs(t); }
    template<typename T>static T hton(T t, const Int2Type<2> &){ return htons(t); }
    template<typename T>static T ntoh(T t, const Int2Type<4> &){ return ntohl(t); }
    template<typename T>static T hton(T t, const Int2Type<4> &){ return htonl(t); }

    public: // for public interface see '(!!!)'

    typedef typename CoreImpl::Buf Buffer;
};
}
}
