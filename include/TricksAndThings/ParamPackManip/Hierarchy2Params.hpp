#pragma once

#include <type_traits>
#include "SignatureChecker.hpp"

struct ParameterListPassed {};
struct ParameterListRequired {};

template<class H, int idx = H::length>  // 'H' (Hierarchy) is to be an instantiation
                                        // of the 'Params2Hierarchy' template;
struct Hierarchy2Params
{
    template<class F, typename... Params>
    static int call(F f, const H &h, Params &&... params)
    {   // convert a hierarchy of types (parameters
        // list) to a parameter pack (and then pass
        // it to f());
        return Hierarchy2Params<H, idx - 1>::
               call(f,
                    h,
                    h.template field<idx>(),
                    std::forward<Params>(params)...);
    }

    // the following structure is used as a "namespace with
    // private section" and represents a single function (doIt());
    struct ProvideMessage
    {   // providing a readable compile-time error message
        // in case of number of parameters of any function
        // call (represented by f()) does not match the
        // required, that represented by types of the 'H'
        // hierarchy fields sequence;
        template<class F, typename... Args>
        static void doIt(F f, const H &h, Args &&... args)
        {
            // find the highest level of hierarchy that is assignable (that
            // is not of null-type, it's pointer or it's reference)...
            enum { lastAssignable = LastAssignable<H, idx>::value };
            // ...and perform the call (see further) on it;
            Hierarchy2Params<H, lastAssignable>::ProvideMessage::doOnward(f, h, args...);
        }

        template<class F, typename... Args>
        static void doOnward(F f, const H &h, Args &&... args)
        {   // does the following:
            // (*0*) erases a pointer-qualifiers from the current
            //       field of 'H' hierarchy if it's necessary;
            // (*1*) adds it to a parameter pack;
            // (*2*) relays the call to the next hierarchy layer;
            typedef typename H::CoreImpl::template Get<idx>::Field Field;

            CorrectLook<decltype(Field::value),    // (*0*)
                        Field::pointerRequired,
                        Field::referenceRequired> correctLook(h.template field<idx>());

            Hierarchy2Params<H,
                             idx - 1    // (*2*)
                            >::ProvideMessage::doIt(f, h,
                                                    correctLook.value,  // (*1*)
                                                    args...);
        }

        private:

        // some local tools:

        // metaprogram is used to correct a look of providing compile-time
        // error message;
        //
        // mind that hierarchy stores just pointers to values (neither
        // values themselves nor references), so we must clear out the
        // undesirable pointers, where the target field's type is plain;
        template<class Field, bool isPtr, bool isRef> struct CorrectLook;
        //
        template<class Field>
        struct CorrectLook<Field *, true, false>
        {
            // field represents a pointer;
            Field *value;
            CorrectLook(Field *arg) : value(arg){}  // in fact, value itself is not
                                                    // used anywhere at the present
                                                    // time, but it will be better
                                                    // to initialize it - to avoid
                                                    // the surprises in the future;
        };
        //
        template<class Field>
        struct CorrectLook<Field, false, true>
        {
            // field represents a reference;
            typename std::remove_pointer<Field>::type &value;
            CorrectLook(Field arg) : value(*arg){}
        };
        // end of a metaprogram;

        // metaprogram is used to find the highest layer of hierarchy (but up to idx1) to that the value can be
        // assigned;
        template<class H1, int idx1, int = H1::CoreImpl::template Get<idx1>::Field::assignable> struct LastAssignable;
        //
        template<class H1, int idx1>
        struct LastAssignable<H1, idx1, false>
        {   // field is not assignable -
            enum
            {   // - so relay one step down and do the same;
                value = LastAssignable<H1, idx1 - 1>::value
            };
        };
        //
        template<class H1, int idx1>
        struct LastAssignable<H1, idx1, true>
        {   // border case - available field is found;
            enum { value = idx1 };
        };
        //
        template<class H1>
        struct LastAssignable<H1, 0, true>
        {   // other border case - no assignable
            // field was found at all, value of
            // 0 signals the bottom of recursion;
            enum { value = 0 };
        };
        //end of a metaprogram;
    };
};

template<class H>
struct Hierarchy2Params<H, 0>
{   // border case - a new parameter pack is built now, so...
    template<class F, typename... Params>
    static int call(F f, const H &, Params &&... params)
    {    // ...call a payload f() with it;
        SignatureChecker<int, Params...> check(f);  // helps to reduce an error message
                                                    // in case of users's error whithin
                                                    // payload function signature;
                                                    // doesn't detects std::bind-wrapped
                                                    // functions errors!!

        return f(params...);
    }

    struct ProvideMessage
    {   // ...provide a compile-time error message;
        template<class F, typename... Args>
        static int doOnward(F f, const H &h, Args &&... args){ doIt(f, h, args...); }

        template<class F, typename... Args>
        static int doIt(F f, const H &, Args &&...)
        {
            struct CannotBeSame {};
            static_assert(std::is_same<CannotBeSame, F>::value,
                          "\n\n\tSize of a parameter list does not matches the one that's required! Search for the"
                          "\n\ttags 'ParameterListPassed' and 'ParameterListRequired' at the following error message;"
                          "\n\tsearch also for 'required from here' at the previous message to find an error point; \n");
            using RequiredArgs = ParameterListRequired(Args...);
            RequiredArgs *requiredArgs = f;
        }
    };
};
