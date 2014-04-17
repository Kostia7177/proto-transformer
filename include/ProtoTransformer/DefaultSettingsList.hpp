#pragma once

#include "detail/PreSettings.hpp"

namespace ProtoTransformer
{

// You can replace this typedef by Your own one - to define
// default settings that are most requently used in Your
// project. So that it is why it moved out from 'detail'.
// Be aware mistakes (to forget something, for example) -
// hundred-screen-long error messages are guaranteed!
typedef Params2Hierarchy
    <
        // proto describing components:
        //  -- whole session (surprise! :) )
        SessionHdrIs<NullType>,
        //
        //  -- request
        RequestHdrIs<PureHdr>,  // by default is uint32_t and is to be calculated
                                // automatically as a size of request data buffer;
                                // see the PureHdr.hpp file;
        GetSizeOfRequestFromHdrIs<Network2HostLong>,    // by default is ntohl();
        SetSizeOfRequest2HdrIs<Host2NetworkLong>,       // by default is htonl();
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
        AnswerHdrIs<PureHdr>,               // the following is the same as for
                                            // request, but for answer;
        SetSizeOfAnswer2HdrIs<Host2NetworkLong>,
        GetSizeOfAnswerFromHdrIs<Network2HostLong>,
        AnswerCompletionIs<NullType>,
        AnswerDataReprIs<unsigned char>,

        // non-proto things:
        NumOfWorkersIs<Int2Type<hardwareConcurrency>>,  // ...but You can set some
                                                        // a bit more smart;
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
        //
        SessionManagerIs<EmptyManager>, // empty (by default) starts the session
                                        // and forget about it - session will be
                                        // dropped when it'll be finished independently
                                        // of server; see SessionManagers/Empty.hpp;
                                        // alternatively there could be used a manager
                                        // with map that remembers all the sessoins and
                                        // terminates them on server's exit; see
                                        // SessionManagers/WithMap.hpp;
        //
        ThreadPoolIs<boost::threadpool::pool>,
        ReadingManagerIs<ReadingManager>    // that thing, that manages the reading
                                            // when no request size is known (not a
                                            // completion function! just what it calls!
                                            // see ReadingManager.hpp file);
        //
    > DefaultSettingsList;

}
