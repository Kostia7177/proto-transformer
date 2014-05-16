#include "ExampleFeatures.hpp"

int InitAnySessionSpecific::operator() (
    const AnySessionHdr &sessionHdr,
    AnySessionSpecific &sessionSpecific)
{
    sessionSpecific.numOfRequestsLeft = ntohl(*getNum(sessionHdr));
    sessionSpecific.startedAt = time(0);

    return 1;
}
