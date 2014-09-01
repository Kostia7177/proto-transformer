#pragma once

#include <ProtoTransformer/Proto.hpp>
#include "ExampleFeatures.hpp"

namespace ProtoTransformer
{

typedef Proto
    <
        UsePolicy<SessionHdrIs, AnySessionHdr>,
        UsePolicy<AnswerHdrIs, AnyAnswerHdrWrapped>
    > ProtoWithSessionHdr;

typedef ProtoWithSessionHdr::RequestData RequestData;
typedef ProtoWithSessionHdr::AnswerData AnswerData;

}
