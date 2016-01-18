#pragma once

#include "../Tools.hpp"
#include "../Session/SessionContext.hpp"
#include "../../../TricksAndThings/SignatureManip/filteringAdapter.hpp"
#include "../../../TricksAndThings/Tools/GccBug47226Satellite.hpp"
#include<boost/asio.hpp>

namespace ProtoTransformer
{

template<class T, class Base = NullType>
struct RequestTimeoutIs : virtual Base
{
    class RequestTimeout
    {
        boost::asio::deadline_timer timer;

        template<class C> static One withAction(typename C::Action *);
        template<class> static Two withAction(...);

        template<class C, typename H>
        static int actionSw(const Int2Type<true> &, H &params)  // (*gccBug47226*), hierarchy 'H'
                                                                // is to be replaced with a parameter
                                                                // pack, the same as at (**);
        {
            typename C::Action action;
            return TricksAndThings::Hierarchy2Params<H>::call(action, params);   // (*gccBug47226**),
                                                                // 'Hierarchy2Params' also is
                                                                // to be replaced
                                                                // with 'filteredHierarchy()'
        }
        
        template<class, typename H>
        static int actionSw(const Int2Type<false> &, H &) { return 1; }  // (**)

        public:

        RequestTimeout(boost::asio::io_service &ioService) : timer(ioService) {}
        ~RequestTimeout() { timer.cancel(); }

        template<class F, typename... Params>
        void set(
            F defaultAction,
            Params &... params)
        {
            int expiresAfter = TricksAndThings::filteringAdapter(&T::value, params...);
            timer.expires_from_now(boost::posix_time::milliseconds(expiresAfter));

            GccBug47226Satellite bugOverriding;
            TricksAndThings::Params2Hierarchy<TricksAndThings::BindNotNullsOnly, Params...> hierarchizedParams(params...);
            GccBug47226Satellite endOfBugOverriding;

            timer.async_wait([=](const boost::system::error_code &errorCode)
                             {
                                if (errorCode != boost::asio::error::operation_aborted
                                    && actionSw<T>(Int2Type<sizeof(withAction<T>(0)) == sizeof(One)>(),
                                                   hierarchizedParams)) // (*gccBug47226*), replace hierarchy
                                                                               // with a parameter pack;
                                {
                                    defaultAction();
                                }
                             });
        }

        void cancel() { timer.cancel(); }
    };

    enum { proto = 0 };
};

template<class Base>
struct RequestTimeoutIs<NullType, Base> : virtual Base
{
    struct RequestTimeout
    {
        template<class T> RequestTimeout(T &) {}

        // hey!! in case of using (once upon a time) an F-type parameter
        // inside the set(...) template function, don't forget to
        // ---------- replace a reference to F with an F's value!!! ----------
        template<class F, typename... Params> void set(const F &, Params &...) {}
        // ... 'cos the reference is an optimisation of an unused copy;
        //
        void cancel() {}
    };

    enum { proto = 0 };
};

}
