#pragma once

#include "../TricksAndThings/ThreadPool/ThreadPool.hpp"
#include "../TricksAndThings/ParamPackManip/Params2TypesHierarchy.hpp"
#include "detail/Configurator/CfgComponents.hpp"
#include "detail/ReadUntilNull.hpp"
#include "detail/AnswerCases.hpp"
#include "detail/SessionManagers/WithMap.hpp"
#include "detail/SessionManagers/Empty.hpp"
#include "detail/StopServerOnSigint.hpp"
#include "detail/JustSize.hpp"
#include "detail/ReadingManager.hpp"
#include "detail/Loggers/Syslog.hpp"
#include "detail/Loggers/Stderr.hpp"

namespace ProtoTransformer
{

// You can replace this typedef by Your own one - to define
// default settings that are most frequently used in Your
// project. So that it is why it moved out from 'detail'.
// Be carefull, avoid the dumb mistakes (to forget something,
// for example) - hundred-screen-long error messages are guaranteed!
typedef TricksAndThings::Params2TypesHierarchy
    <
        // proto describing components:
        //  -- whole session (surprise! :) )
        SessionHdrIs<NullType>,
        //
        //  -- request
        RequestHdrIs<JustSize>,  // by default is uint32_t and is to be calculated
                                // automatically as a size of request data buffer;
                                // see the JustSize.hpp file;
        //
        RequestCompletionIs<NullType>,  // user can specify a request-completion
                                        // function instead of request header;
                                        // note that the request header must be
                                        // turned to NullType in this case
                                        // explicitly;
        RequestDataReprIs<unsigned char>,   // value type of a request vector;
        //
        //  -- answer
        ServerSendsAnswer<AtLeastHeader>,   // by default - if answer contains no
                                            // data, just a header will be returned
                                            // to a client (signalling that no data
                                            // will follow);
                                            // alternately, there is case when no
                                            // answer supposed at all - requests-only
                                            // proto, and a case sends nothing if
                                            // no answer data - even header;
                                            // see AnswerCases.hpp file;
        //
        AnswerHdrIs<JustSize>,               // the following is the same as for
                                            // request, but for answer;
        AnswerCompletionIs<NullType>,
        AnswerDataReprIs<unsigned char>,

        // non-proto things:
        NumOfWorkersIs<Int2Type<hardwareConcurrency>>,  // ...but You can set something
                                                        // a bit more smart;
        ParallelRequestsPerSessionIs<Int2Type<1>>,  // has a meaning if SessionThreadPoolIs
                                                    // points to something multi-threaded
                                                    // (boost::threadpool, for example)
                                                    // note, that 0 means thread concurrency!
        //
        SessionSpecificIs<NullType>,    // such a session-static variable(s); some
                                        // data that is available for all requests
                                        // within a session (and is not available
                                        // for all the other sessions requests);
                                        // see examples/suchATricky1/ for a use-case;
        //
        InitSessionSpecificIs<NullType>,    // function that initializes session
                                            // specific (may be by session-header
                                            // data or it's part);
        ServerSpaceIs<NullType>,
        ClientGlobalSpaceIs<NullType>,
        //
        SessionManagerIs<EmptyManager>, // empty (by default); starts the session
                                        // and forget about it - session will be
                                        // dropped when it'll be finished independently
                                        // of a server; see SessionManagers/Empty.hpp;
                                        // alternatively there could be used a manager
                                        // with map that remembers all the sessions and
                                        // terminates them on server's exit; see
                                        // SessionManagers/WithMap.hpp;
        //
        ServerThreadPoolIs<TricksAndThings
                           ::ThreadPool<TricksAndThings
                                        ::shutdownImmediate>>,
        SessionThreadPoolIs<NullType>,
        LoggerIs<NullType>,
        RequestTimeoutIs<NullType>,
        AnswerTimeoutIs<NullType>,
        SigintHandlerIs<NullType>,
        ReadingManagerIs<ReadingManager>    // that thing, that manages the reading
                                            // when no request size is known (not a
                                            // completion function! just what it calls!
                                            // see ReadingManager.hpp file);
        //
    > DefaultSettingsList;

}
