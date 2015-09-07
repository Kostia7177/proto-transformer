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

int doSomething(const AnyHdr &, const RequestData &, AnswerData &);

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
            UsePolicy<SessionManagerIs, SessionManagerWithMap>,
            UsePolicy<LoggerIs, SyslogLogger>,
            UsePolicy<SigintHandlerIs, StopServerOnSigint>
        > ServerInstance;
    ServerInstance server(port, doSomething);

    return 0;
}

int doSomething(
    const AnyHdr &hdr,
    const RequestData &inBuffer,
    AnswerData &outBuffer)
{
    uint32_t hdrOpt = hdr.get<optField>();
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
    answer.set<textField>("At thread id ");
    answer.get<intField>()[0] = pthread_self();
    answer.set<modeField>('x');
    answer.set<numOfInts>(1);
    outBuffer.push_back(answer);
    answer.set<textField>("i/o buffers ");
    answer.get<intField>()[0] = (unsigned long int)&inBuffer;
    answer.get<intField>()[1] = (unsigned long int)&outBuffer;
    answer.set<modeField>('x');
    answer.set<numOfInts>(2);
    outBuffer.push_back(answer);
    answer.set<textField>("started at ");
    answer.get<intField>()[0] = time(0);
    answer.set<modeField>('t');
    answer.set<numOfInts>(1);
    outBuffer.push_back(answer);

    if (hdrOpt == sumNumbers)
    {
        answer.set<textField>("Sum : ");
        answer.get<intField>()[0] = accumulate(inBuffer.begin(), inBuffer.end(), 0);
        answer.set<numOfInts>(1);
    }
    else
    {
        RequestData ret(inBuffer);
        const char *optStr = "Echo";
        if (hdrOpt == sortNumbers)
        {
            std::sort(ret.begin(), ret.end());
            optStr = "Sorted";
        }
        answer.set<textField>(optStr);
        size_t idx = 0;
        for (uint32_t retNum : ret)
        {
            if (idx == decltype(answer.get<intField>())::size) { break; }
            answer.get<intField>()[idx ++ ] = retNum;
        }
        answer.set<numOfInts>(idx);
    }
    answer.set<modeField>('d');
    outBuffer.push_back(answer);
    struct timespec pause = { hdr.get<pauseField>(), 0 };
    nanosleep(&pause, 0);
    return 1;
}
