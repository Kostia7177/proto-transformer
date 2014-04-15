#pragma once

#include "detail/PreSettings.hpp"

namespace ProtoTransformer
{

typedef Params2Hierarchy
    <
        // proto describing components:
        //  -- whole session (surprise! :) )
        SessionHdrIs<NullType>,
        //  -- request
        RequestHdrIs<PureHdr>,
        GetSizeOfRequestFromHdrIs<Network2HostLong>,
        SetSizeOfRequest2HdrIs<Host2NetworkLong>,
        RequestCompletionIs<NullType>,
        RequestDataReprIs<unsigned char>,
        //  -- answer
        ServerSendsAnswer<AtLeastHeader>,
        AnswerHdrIs<PureHdr>,
        SetSizeOfAnswer2HdrIs<Host2NetworkLong>,
        GetSizeOfAnswerFromHdrIs<Network2HostLong>,
        AnswerCompletionIs<NullType>,
        AnswerDataReprIs<unsigned char>,

        // non-proto things:
        NumOfWorkersIs<Int2Type<hardwareConcurrency>>,
        SessionSpecificIs<NullType>,
        InitSessionSpecificIs<NullType>,
        SessionManagerIs<EmptyManager>,
        ThreadPoolIs<boost::threadpool::pool>,
        ReadingManagerIs<ReadingManager>
    > DefaultSettingsList;

}
