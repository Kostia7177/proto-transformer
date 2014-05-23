#pragma once

// hi, stranger!

// the function You may looking for ('filteringAdapter(...)', isn't
// it?) is at the very end of the file - start learning it from (*)
// and pass through the file backwards. no comments will needed at
// all! sure.

#include <type_traits>

// ...if You are still reading stright ahead, so -

// the following is a 'working body' for the
// 'Params2FilteredHierarchyCore' see (**)
template<int idx>
struct BinderPos
{
    // (****) 'length' plays a double role. it
    // calculates an index that will be passed
    // to the next field, and - at the last field
    // of hierarhy - equals an 'actual length' of
    // it (number of meaningfull fields it contains)
    enum { length = idx, pos = length };
};
template<typename T>
struct BinderData
{
    typedef T Type;
    T value;

    BinderData(T arg) : value(arg) {}

    // 'Get' and 'get' are accessible at the last
    // field of hierarchy - see (***). within all
    // other cases it is hiden by same name
    // whithin derived;
    template<int> struct Get { typedef T Type; };
    template<int> T get() { return value; }
};
template<int idx, typename T, int uniquizer>
struct Binder
    : BinderData<T &>,
      BinderPos<idx + 1>
{   // common case; puts a reference to 'T' as
    // a payload field;
    Binder(T &arg) : BinderData<T &>(arg) {}
};

template<int idx, typename Pointee, int uniquizer>
struct Binder<idx, Pointee *, uniquizer>
    : BinderData<Pointee *>,
      BinderPos<idx + 1>
{   // pointers must be represented as is,
    // not as referenses;
    Binder(Pointee *arg) : BinderData<Pointee *>(arg) {}
};

template<int idx, int uniquizer>
struct Binder<idx, NullType, uniquizer>
    : BinderPos<idx>
{   // hiding the meanless 'NullType'
    Binder(NullType) {}
};

template<int idx, int uniquizer>
struct Binder<idx, const NullType, uniquizer>
    : BinderPos<idx>
{
    Binder(NullType) {}
};

template<int idx, int uniquizer>
struct Binder<idx, NullType *, uniquizer>
    : BinderPos<idx>
{
    Binder(NullType *) {}
};

// (**)
// the following structure converts any type sequence, represented by
// it's variadic template parameters up from third, to any 'filtered'
// hierarchy. why 'filtered' : fields, according to the 'NullType'
// members of the input sequence will be hidden and inaccessible for
// the client's code; i.e. hierarchy 'filters out' all the 'NullType'
// fields via the 'Binder' template; so, only 'meaningfull' types
// will be represented whithin the output product;

// the common case cannot contain anything but forward declaration;
template<int, int, typename...> struct Params2FilteredHierarchyCore;

template<int idx, int uniquizer, typename Last>
struct Params2FilteredHierarchyCore<idx, uniquizer, Last>
{   // border case - variadic template params
    // are exhausted; put it's last member into
    // the output hierarchy, stop the recursion
    // and enjoy;
    typedef Binder<idx, Last, uniquizer> Type;
};

template<int idx, int uniquizer, typename Head, typename... Tail>
class Params2FilteredHierarchyCore<idx, uniquizer, Head, Tail...>
{   // main working case
    typedef Binder<idx, Head, uniquizer> BinderLocal;   // will be a current field;

    enum { nextPos = BinderLocal::length };
    typedef typename Params2FilteredHierarchyCore<nextPos, uniquizer + 1, Tail...>::Type FollowingFields;
                                                                        //   ^^^^^^^^^^^^^^^^^^^^^^^^^^ :)
    public:                                                             //   |
    struct Type                                                         //   |
        : BinderLocal,      // current field - placing (or not placing in    |
                            // case of 'NullType') the current type into     |
                            // the hierarchy;                                |
          FollowingFields   // all the other fields - continuing the         |
                            // compiler's recursion ------------------------>|
    {
        // (***)
        // the following structure needed for returning value type deducing
        // of Params2FilteredHierarchy::field<pos>() and it's intermediates
        // get() and getSw();
        template<int arg, class = Int2Type<arg == BinderLocal::pos>> struct Get;

        template<int arg>
        struct Get<arg, Int2Type<false>>
        {
            typedef typename FollowingFields::template Get<arg>::Type Type;
        };

        template<int arg>
        struct Get<arg, Int2Type<true>>
        {
            typedef typename BinderLocal::Type Type;
        };

        // actually sees at the last field of hierarchy (border case), where
        // nested Type is Binder (in other words, where Binder::length is not
        // redefined by (derived class Type)::length);
        enum { length = FollowingFields::length };

        Type(Head &head, Tail &... tail) : BinderLocal(head), FollowingFields(tail...) {}

        template<int pos>
        typename Get<pos>::Type get()
        {
            return getSw<pos>(Int2Type<pos == BinderLocal::pos>()); 
        }

        private:
        template<int pos> typename Get<pos>::Type getSw(const Int2Type<false> &){ return FollowingFields::template get<pos>(); }
        template<int pos> typename Get<pos>::Type getSw(const Int2Type<true> &){ return BinderLocal::value; }
    };
};

template<typename... Params>
class Params2FilteredHierarchy
{   // wrapping the previous structure to hide a service things
    // like the first 2 template parameters (integers)
    typedef typename Params2FilteredHierarchyCore<0, 0, Params...>::Type Core;
    Core core;

    public:
    Params2FilteredHierarchy(Params &... params) : core(params...) {}
    enum { length = Core::length };
    // accessing the hierarchy's field number 'pos' (starts with 1)
    template<int pos> typename Core::template Get<pos>::Type field(){ return core.template get<pos>(); }
};

template<class RetType, class... Params>
struct SignatureChecker
{
    template<class F>
    SignatureChecker(F *f2Check)
    {
        using FGood = RetType(Params...);

        static_assert(std::is_same<FGood, F>::value,
                      "\n\n\tSignature of a function does not match the context types requirements! "
                      "\n\tFor details see the following error message. \n");
        FGood *fGood = f2Check;
    }

    template<class PassedAsFunctor>
    SignatureChecker(PassedAsFunctor)
    {
        // sadly, doesn'n detect errors whithin std::bind-wrapped functions;
        // just whithin simple functors with operator();
        // have no idea why...
        using DefinedByContextTypesOperator = RetType(PassedAsFunctor::*)(Params...);
        typedef char One;
        struct Two { One two[2]; };
        One test(DefinedByContextTypesOperator);
        Two test(...);
        static_assert(sizeof(test(&PassedAsFunctor::operator())) == sizeof(One),
                      "\n\n\tSingature of a functor does not match the context types requirements! "
                      "\n\tFor details see the following error message. \n");
        DefinedByContextTypesOperator definedByContextTypesOperator = &PassedAsFunctor::operator();
    }
};

template<class ParamsList, int idx = ParamsList::length>
struct Hierarchy2Params
{   // convert a hierarchy of parameters list to a variadic;
    template<class F, typename... Params>
    static int call(F f, ParamsList &paramsList, Params &&... params)
    {
        return Hierarchy2Params<ParamsList, idx - 1>::
               call(f,
                    paramsList,
                    paramsList.template field<idx>(),
                    std::forward<Params>(params)...);
    }
};

template<class ParamsList>
struct Hierarchy2Params<ParamsList, 0>
{   // border case - a new variadic is built now, so
    // call a payload with it;
    template<class F, typename... Params>
    static int call(F f, ParamsList &, Params &&... params)
    {
        SignatureChecker<int, Params...> check(f);  // helps to reduce an error message
                                                    // in case of users's error whithin
                                                    // payload function signature;
                                                    // doesn't detects std::bind-wrapped
                                                    // functions errors!!

        return f(params...);
    }
};

template<class F, typename... Params>
int filteringAdapter(F f, Params &... params)
{   // (*) so - here is that, for what the all previous ummagumma is:
    //
    // -- aim:
    //
    // suppose there is a fixed-size set of variable types:
    //
    //  t1       t2       t3       t4       t5
    //
    // that are defined inside any traits structure. any of theese types
    // can be instantiated by an 'empty' NullType, and any others - by
    // some meaningfull Ti:
    //
    //  T1       NullType T3       T4       NullType    -- (ex 1)
    //  NullType NullType NullType NullType T5          -- (ex 2)
    //  T1       T2       NullType T4       NullType    -- (ex 3)
    //  NullType T2       NullType NullType T5          -- (ex 4)
    //
    // suppose now, that we want to pass the objects of theese types to any client's
    // function, which knows nothing about our 'NullType' (and, worst of all, don't
    // want to know).
    // for our four cases of theese client's functions are:
    //
    //  int f1(T1, T3, T4);     // ex 1
    //  int f2(T5);             // ex 2
    //  int f3(T1, T2, T4);     // ex 3
    //  int f4(T2, T5);         // ex 4
    //
    // so, all we've got to do is to filter out all the NullType-objects and to pass
    // the filtered set to a client's code.
    //
    // -- implementation:
    //
    // 'Params...' contains the input (dirty) set of types;
    typedef Params2FilteredHierarchy<Params... > ParamsList;
    ParamsList paramsList(params... );  // simply creating a 'filtered hierarchy'...
    // ...and simply building a new, filtered, variadic. And then simply call a payload,
    return Hierarchy2Params<ParamsList>::call(f, paramsList);
    // ...and nothing more.
}
