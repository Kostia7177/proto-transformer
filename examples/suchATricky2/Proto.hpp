#pragma once

#include <ProtoTransformer/Proto.hpp>
#include "ExampleFeatures.hpp"

namespace ProtoTransformer
{

typedef Proto
    <
        UsePolicy<RequestHdrIs, AnyHdr>,
        UsePolicy<GetSizeOfRequestFromHdrIs, GetRequestSize>,
        UsePolicy<SetSizeOfRequest2HdrIs, SetRequestSize>,
        UsePolicy<ServerSendsAnswer, NothingIfNoData>,
        UsePolicy<RequestDataReprIs, uint32_t>,
        UsePolicy<AnswerDataReprIs, Answer>
    > ProtoWithAnyHdr;

typedef typename ProtoWithAnyHdr::RequestData RequestData;
typedef typename ProtoWithAnyHdr::AnswerData AnswerData;

}
