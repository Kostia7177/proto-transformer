#pragma once

#include "../Tools.hpp"
#include "Hierarchy2Params.hpp"

namespace ProtoTransformer
{

template<template<int, typename> class Bind, typename... Params>
class Params2Hierarchy
{   // the following metaprogram converts any type sequence, represented by
    // it's template parameter pack, to an hierarchy.
    // the common case cannot contain anything but forward declaration;
    template<
                template<int, typename> class,      // binder template class (binds a value
                                                    // itself to a point of hierarchy), one
                                                    // of Binders/*.hpp (with exception of
                                                    // Binders/Bases.hpp, whitch are must
                                                    // (BinderPos) or can (BinderData) be
                                                    // public bases for any binder);
                int,    // 'meaningfull' index of a field (fields that are to be filtered out,
                        // of type NullType, has equal indexes, look for (*assignable length*)
                        // at the comments);
                typename... // type sequence (that is to be converted to hierarchy) as is;
            > struct Core;
    //
    template<template<int, typename> class Bind1, int idx>
    struct Core<Bind1, idx>
    {   // (*border case*) - parameters pack
        // is exhausted; put it's length
        // into the output hierarchy;
        struct Type
        {
            void populate(){}
            enum { length = idx };
        };
    };
    //
// while (!(*border case*)) // ...compile-time, of course;
// {
    template<template<int, typename> class Bind1,
             int idx,
             typename Head,
             typename... Tail>
    struct Core<Bind1, idx, Head, Tail...>
    {   // main working case
        typedef Bind1<idx, Head> Field;   // will be a current field;
        enum { nextPos = Field::length };
        typedef Core<Bind1, nextPos, Tail...> FollowingFields;   // <---------------------------+
        // the following structure needed for returning value type deduction                    ^
        // of Params2Hierarchy::field<pos>() and it's intermediates                             |
        // get() and getSw();                                                                   |
        template<int arg, int = arg == Field::pos> struct Get;  //                              |
        //                                                                                      |
//   while(!(*field found*)) // compile-time again;                                             |
//   {                                                                                          |
        template<int arg>   //                                                                  |
        class Get<arg, false>  //                                                               |
        {   //                                                                                  |
            typedef typename FollowingFields    //                                              |
                             ::template Get<arg> GetNext;   //                                  |
                                                                    //                          |
            public: //                                                                          |
                            //                                                                  |
            typedef typename GetNext::Field Field;  //                                          |
            typedef typename GetNext::RetType RetType; //                                       |
        };  //                                                                                  |
//   }                                                                                          |
        //                                                                                      |
        template<int arg>   //                                                                  |
        struct Get<arg, true>   //                                                              |
        {   // (*field found*) - border case;                                                   |
            typedef Core::Field Field;  //                                                      |
            typedef decltype(Field::value) RetType;  //                                         |
        };  //                                                                                  |
        //                                                                                      |
        class Type   //                                                                         |
        {   //                            //                                                    |
            Field field;        // current field - placing (or not placing in                   |
                                // case of 'NullType' and Bind == NotNullsOnly)                 |
                                // the current type into the hierarchy;                         |
            typename FollowingFields::Type tail;  // all the other fields except current -      |
                                                  // continuing the compiler's recursion ------>+
            // a pair of sfinae-switchers:
            typedef const Int2Type<false> GoForw;
            typedef const Int2Type<true> ThisField;
            //
            template<int pos>
            typename Get<pos>::RetType getSw(GoForw &) const     { return tail.get<pos>(); }
            //
            template<int pos>
            typename Get<pos>::RetType getSw(ThisField &) const  { return field.value; }

            typedef const Int2Type<false> PassBy;
            typedef const Int2Type<true> AssignField;
            //
            template<typename... Args>
            void setSw(
                PassBy &,
                Args &&... args)
            {
                tail.populate(std::forward<Args>(args)...);
            }
            //
            template<typename FirstArg, typename... Args>
            void setSw(
                AssignField &,
                FirstArg &&firstArg,
                Args &&...args)
            {
                field.set(firstArg);
                tail.populate(std::forward<Args>(args)...);
            }

            public:

            // 'length' actually sees at the last field of
            // an hierarchy - see (*border case*);
            enum { length = FollowingFields::Type::length };

            Type(Head &h, Tail &... t) : field(h), tail(t...) {}
            Type(){}

            template<int pos>
            typename Get<pos>::RetType get() const
            {
                enum { thisField = pos == Field::pos };
                return getSw<pos>(Int2Type<thisField>());
            }

            template<typename... Args>
            void populate(Args &&... args)
            {
                setSw(Int2Type<Field::assignable>(),
                      args...);
            }

            void populate(){}
        };
    };
// }
    // end of a metaprogram;

    public:
    typedef Core<Bind, 0, Params...> CoreImpl;
    private:

    typename CoreImpl::Type core;

    // the following metaprogram is a tool to calculate an (*assignable length*) - a number of
    // assignable fields at a hierarchy; inassignable fields are fields of type 'NullType'
    // (either it's pointer or it's reference), or proxies for 'JustSize';
    //
    // sfinae-based detector of the end of a hierarchy;
    template<class C> static One atLastField(typename C::FollowingFields::Field *);
    template<typename> static Two atLastField(...);
    //
    template<class HierarchyLevel,
             int accumulated = 0,   // input (for the current level) value, 0 on the beginning;
             // output value (that is to be passed to the next level);
             int accumulatedNext = accumulated + HierarchyLevel::Field::assignable,
             bool borderCaseDetector = sizeof(Params2Hierarchy::atLastField<HierarchyLevel>(0))
                                       == sizeof(Two)
            > struct NumOfAssignables;
    //
    template<class HierarchyLevel, int accumulated, int accumulatedNext>
    struct NumOfAssignables<HierarchyLevel, accumulated, accumulatedNext, false>
    {   // main working case - scanning hierarchy's body
        // and go forward;
        static const int value = NumOfAssignables<typename HierarchyLevel::FollowingFields,
                                                  accumulatedNext
                                                 >::value;
    };
    //
    template<class HierarchyLastLevel, int accumulated, int accumulatedNext>
    struct NumOfAssignables<HierarchyLastLevel, accumulated, accumulatedNext, true>
    {   // border case - hierarchy is looked through and
        // value is calculated;
        static const int value = accumulatedNext;
    };
    // end of a metaprogram;

    typedef const Int2Type<false> SizeWrong;
    typedef const Int2Type<true> SizeOk;

    template<typename... Args>
    void populateSw(SizeOk &, Args &&... args)      // if length of parameter pack equals to
                                                    // (*assignable length*) of the hierarchy
    { core.populate(std::forward<Args>(args)...); } // - populate the hierarchy;

    template<typename... Args>
    void populateSw(SizeWrong &, Args &&... )       // else
    {                                               // - report the error;
        using PassedArgs = ParameterListPassed(Args...);
        PassedArgs *passedArgs = 0;
        Hierarchy2Params<Params2Hierarchy>::ProvideMessage::doIt(passedArgs, *this);
    }

    public:

    Params2Hierarchy(Params &... params) : core(params...) {}
    Params2Hierarchy(){}

    enum { length = CoreImpl::Type::length };

    // accessing the hierarchy's field number 'pos' (starts with 1) for reading...
    template<int pos> typename CoreImpl::template Get<pos>::RetType field() const
    { return core.template get<pos>(); }

    // ...and for writing;
    template<typename... Args> void populate(Args &&... args)
    {
        enum { sizeOfParamPackMatching = NumOfAssignables<CoreImpl>::value == sizeof...(Args) };
        populateSw(Int2Type<sizeOfParamPackMatching>(), std::forward<Args>(args)...);
    }
};
}
