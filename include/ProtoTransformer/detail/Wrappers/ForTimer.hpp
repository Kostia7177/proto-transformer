#pragma once

#include "../Tools.hpp"
#include "../../../TricksAndThings/ParamPackManip/filteringAdapter.hpp"
#include "../Session/SessionContext.hpp"
#include "../GccBug47226Satellite.hpp"
#include <boost/asio.hpp>

namespace ProtoTransformer
{
namespace Wrappers
{

template<class Timeout>
struct ForTimer
{
    class TimerItself
    {
        boost::asio::deadline_timer timer;

        template<class T> static One withAction(typename T::Action *);
        template<class> static Two withAction(...);

        template<class T, typename H>
        static int actionSw(const Int2Type<true> &, H &params)  // (*gccBug47226*), hierarchy 'H'
                                                                // is to be replaced with a parameter
                                                                // pack, the same as at (**);
        {
            typename T::Action action;
            return Hierarchy2Params<H>::call(action, params);   // (*gccBug47226**),
                                                                // 'Hierarchy2Params' also is
                                                                // to be replaced
                                                                // with 'filteredHierarchy()'
        }
        
        template<class, typename H>
        static int actionSw(const Int2Type<false> &, H &) { return 1; }  // (**)

        public:

        TimerItself(boost::asio::io_service &ioService) : timer(ioService) {}
        ~TimerItself() { timer.cancel(); }

        template<class F, typename... Params>
        void set(
            F defaultAction,
            Params &... params)
        {
            int expiresAfter = filteringAdapter(&Timeout::value, params...);
            timer.expires_from_now(boost::posix_time::milliseconds(expiresAfter));

            GccBug47226Satellite bugOverriding;
            Params2Hierarchy<BindNotNullsOnly, Params...> hierarchizedParams(params...);
            GccBug47226Satellite endOfBugOverriding;

            timer.async_wait([=](const boost::system::error_code &errorCode)
                             {
                                if (errorCode != boost::asio::error::operation_aborted
                                    && actionSw<Timeout>(Int2Type<sizeof(withAction<Timeout>(0)) == sizeof(One)>(),
                                                         hierarchizedParams))   // (*gccBug47226*), replace hierarchy
                                                                                // with a parameter pack;
                                {
                                    defaultAction();
                                }
                             });
        }

        void cancel() { timer.cancel(); }
    };
};

template<>
struct ForTimer<NullType>
{
    struct TimerItself
    {
        template<class T> TimerItself(T &) {}

        // hey!! in case of using (once upon a time) an F-type parameter
        // inside the set(...) template function, don't forget to
        // ---------- replace a reference to F with an F's value!!! ----------
        template<class F, typename... Params> void set(const F &, Params &...) {}
        // ... 'cos the reference is an optimisation of an unused copy;
        //
        void cancel() {}
    };
};

}
}
