#pragma once

#include <ProtoTransformer/Proto.hpp>

namespace ProtoTransformer
{

typedef Proto
    <
        UsePolicy<RequestCompletionIs, ReadUntilNull>,
        UsePolicy<RequestHdrIs, NullType>,
        UsePolicy<RequestDataReprIs, char>,
        UsePolicy<ServerSendsAnswer, NothingIfNoData>,
        UsePolicy<AnswerCompletionIs, ReadUntilNull>,
        UsePolicy<AnswerHdrIs, NullType>,
        UsePolicy<AnswerDataReprIs, char>
    > ProtoStrings;

typedef typename ProtoStrings::RequestData RequestData;
typedef typename ProtoStrings::AnswerData AnswerData;

}
