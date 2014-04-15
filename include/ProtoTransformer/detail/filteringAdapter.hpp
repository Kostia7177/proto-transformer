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
template<int idx, typename T, int uniquizer>
struct Binder
{   // common case; puts 'T' as a payload field;
    // 'length' plays a double role. it calculates
    // an index that will be passed to the next
    // field, and - at the last field of hierarhy -
    // equals an 'actual length' of it (number of
    // meaningfull fields it contains)
    enum { lenght = idx + 1, pos = lenght };
    typedef T Type;
    Type &value;
    Binder() {}
    Binder(Type &arg) : value(arg) {}
    // 'Get' and 'get' are accessible at the last
    // field of hierarchy - see (***). within all
    // other cases they are hiden by derived;
    template<int> struct Get { typedef T Type; };
    template<int> T &get() { return value; }
};

template<int idx, int uniquizer>
struct Binder<idx, NullType, uniquizer>
{   // hiding the meanless 'NullType'
    enum { lenght = idx, pos = lenght };
    typedef NullType Type;
    Binder(){}
    Binder(NullType){}
    template<int> struct Get { typedef NullType Type; };
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

    enum { nextPos = BinderLocal::lenght };
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
        enum { lenght = FollowingFields::lenght };

        Type(Head &head, Tail &... tail) : BinderLocal(head), FollowingFields(tail...) {}

        template<int pos>
        typename Get<pos>::Type &get()
        {
            typedef Int2Type<pos == BinderLocal::pos
                             && std::is_same<typename BinderLocal::Type,
                                             NullType>::value == false
                             && std::is_same<typename BinderLocal::Type ,
                                             size_t>::value == false> Switcher;
            Switcher switcher;
            return getSw<pos>(switcher); 
        }

        private:
        template<int pos> typename Get<pos>::Type &getSw(const Int2Type<false> &){ return FollowingFields::template get<pos>(); }
        template<int pos> typename Get<pos>::Type &getSw(const Int2Type<true> &){ return BinderLocal::value; }
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
    enum { lenght = Core::lenght };
    // accessing the hierarchy's field number 'pos' (starts with 1)
    template<int pos> typename Core::template Get<pos>::Type &field(){ return core.template get<pos>(); }
};

// a number of overloads of 'expandingAdapter(...)' -
// each one corresponds to the length of it's
// parameter list, that is a 'Params2FilteredHierarchy'
// that described earlier;
// is an intermediate function, used inside of
// 'filteringAdapter(...)' - see (*)
template<class F, class ParamsList>
int expandingAdapter(
    F f,                    // adapting function;
    ParamsList &paramsList, // filtered hierarchy 'Params2FilteredHierarchy';
    Int2Type<1>)            // lenght of parameters list (here is for
                            // 1 parameter);
{
    return f(paramsList.template field<1>());
}

template<class F, class ParamsList>
int expandingAdapter(
    F f,
    ParamsList &,
    Int2Type<0>)            // oh well sorry, of corse...
{
    return f();
}

template<class F, class ParamsList>
int expandingAdapter(
    F f,
    ParamsList &paramsList,
    Int2Type<2>)
{
    return f(paramsList.template field<1>(),
             paramsList.template field<2>());
}

template<class F, class ParamsList>
int expandingAdapter(
    F f,
    ParamsList &paramsList,
    Int2Type<3>)
{
    return f(paramsList.template field<1>(),
             paramsList.template field<2>(),
             paramsList.template field<3>());
}

template<class F, class ParamsList>
int expandingAdapter(
    F f,
    ParamsList &paramsList,
    Int2Type<4>)
{
    return f(paramsList.template field<1>(),
             paramsList.template field<2>(),
             paramsList.template field<3>(),
             paramsList.template field<4>());
}

template<class F, class ParamsList>
int expandingAdapter(
    F f,
    ParamsList &paramsList,
    Int2Type<5>)
{
    return f(paramsList.template field<1>(),
             paramsList.template field<2>(),
             paramsList.template field<3>(),
             paramsList.template field<4>(),
             paramsList.template field<5>());
}

template<class F, class ParamsList>
int expandingAdapter(
    F f,
    ParamsList &paramsList,
    Int2Type<6>)
{
    return f(paramsList.template field<1>(),
             paramsList.template field<2>(),
             paramsList.template field<3>(),
             paramsList.template field<4>(),
             paramsList.template field<5>(),
             paramsList.template field<6>());
}
// ...continue for more number of fields when it
// will be needed;

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
    // ...and simply calling the according to hierarchy's length expanding adapter;
    return expandingAdapter(f, paramsList, Int2Type<ParamsList::lenght>());
    // ...and nothing more.
}
