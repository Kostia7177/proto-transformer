#pragma once

#include "../Tools.hpp"
#include <type_traits>

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
        One test(DefinedByContextTypesOperator);
        Two test(...);
        static_assert(sizeof(test(&PassedAsFunctor::operator())) == sizeof(One),
                      "\n\n\tSingature of a functor does not match the context types requirements! "
                      "\n\tFor details see the following error message. \n");
        DefinedByContextTypesOperator definedByContextTypesOperator = &PassedAsFunctor::operator();
    }
};
