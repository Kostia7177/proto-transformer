#pragma once

#include <ProtoTransformer/Proto.hpp>

namespace ProtoTransformer
{

typedef Proto<> ProtoSimplest;

typedef typename ProtoSimplest::RequestData RequestData;
typedef typename ProtoSimplest::AnswerData AnswerData;

}
