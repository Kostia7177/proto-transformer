#pragma once

#include <ProtoTransformer/Proto.hpp>
#include "ExampleFeatures.hpp"

namespace ProtoTransformer
{

typedef Proto
    <
        UsePolicy<RequestHdrIs, AnyHdrWrapped>,
        UsePolicy<RequestDataReprIs, uint32_t>,
        UsePolicy<ServerSendsAnswer, NothingIfNoData>,
        UsePolicy<AnswerDataReprIs, Answer>
    > ProtoWithAnyHdr;

typedef typename ProtoWithAnyHdr::RequestData RequestData;
typedef typename ProtoWithAnyHdr::AnswerData AnswerData;

}
