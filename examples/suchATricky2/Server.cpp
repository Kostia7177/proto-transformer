#include <ProtoTransformer/Server.hpp>
#include "Proto.hpp"
#include <algorithm>
#include <sstream>

int usage(
    int ret,
    char *argv0)
{
    (ret ? std::cerr : std::cout) << "usage: " << argv0 << " <port> " << std::endl;
    return ret;
}

using namespace ProtoTransformer;

int doSomething(AnyHdr &, const RequestData &, AnswerData &);

int main(
    int argc,
    char **argv)
{
    if (argc < 2) { return usage(1, argv[0]); }
    int port = strtol(argv[1], 0, 0);
    if (!port)
    {
        std::cerr << "Cannot get a port number from " << argv[1] << "; " << std::endl;
        return 1;
    }

    typedef Server
        <
            ProtoWithAnyHdr,
            UsePolicy<SessionManagerIs, SessionManagerWithMap>
        > ServerInstance;
    ServerInstance server(port, doSomething);

    return 0;
}

int doSomething(
    AnyHdr &hdr,
    const RequestData &inBuffer,
    AnswerData &outBuffer)
{
    uint32_t hdrOpt = getHdrField(hdr, optFieldIdx);
    if (hdrOpt == terminateSession) { return 0; }
    if (hdrOpt == logRequest)
    {
        std::cout << "Received " << inBuffer.size() << " of (maybe) uint32_t; " << std::endl;
        std::copy(inBuffer.begin(),
                  inBuffer.end(),
                  std::ostream_iterator<RequestData::value_type>(std::cout, ","));
        std::cout << " No sending back any answer; " << std::endl;
        return 1;
    }

    Answer answer;
    snprintf(answer.data, sizeof(answer.data), "At thread id %lu; ", (unsigned long int)pthread_self());
    outBuffer.push_back(answer);
    snprintf(answer.data, sizeof(answer.data), "i/o buffers %#lx/%#lx; ", (unsigned long int)&inBuffer, (unsigned long int)&outBuffer);
    outBuffer.push_back(answer);
    snprintf(answer.data, sizeof(answer.data), "started at %lu; ", time(0));
    outBuffer.push_back(answer);

    if (hdrOpt == sumNumbers)
    {
        snprintf(answer.data, sizeof(answer.data), "Sum : %u; ", accumulate(inBuffer.begin(), inBuffer.end(), 0));
    }
    else
    {
        RequestData ret(inBuffer);
        std::stringstream retStr;
        const char *optStr = "Echo";
        if (hdrOpt == sortNumbers)
        {
            std::sort(ret.begin(), ret.end());
            optStr = "Sorted";
        }
        for (uint32_t retNum : ret)
        {
            retStr << retNum << "; " ;
        }
        snprintf(answer.data, sizeof(answer.data), "%s: %s ", optStr, retStr.str().c_str());
    }
    outBuffer.push_back(answer);
    struct timespec pause = { getHdrField(hdr, pauseFieldIdx), 0 };
    nanosleep(&pause, 0);
    return 1;
}
