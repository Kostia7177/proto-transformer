#pragma once

#include "Tools.hpp"

enum { never, atLeastHdr, nothingIfNoData };
typedef Int2Type<never> NoAnswerAtAll;
typedef Int2Type<atLeastHdr> AtLeastHeader;
typedef Int2Type<nothingIfNoData> NothingIfNoData;
