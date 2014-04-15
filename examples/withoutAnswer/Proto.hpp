#pragma once

#include <ProtoTransformer/Proto.hpp>

namespace ProtoTransformer
{

typedef Proto
    <
        UsePolicy<RequestCompletionIs, ReadUntilNull>,
        UsePolicy<RequestHdrIs, NullType>,
        UsePolicy<RequestDataReprIs, char>,
        UsePolicy<ServerSendsAnswer, NoAnswerAtAll>
    > ProtoWithoutAnswer;

typedef typename ProtoWithoutAnswer::RequestData RequestData;

}
