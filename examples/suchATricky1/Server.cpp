#include <ProtoTransformer/Server.hpp>
#include "Proto.hpp"
#include <set>
#include <sstream>
#include <boost/bind.hpp>
#include <mutex>

int usage(
    int ret,
    char *argv0)
{
    (ret ? std::cerr : std::cout) << "usage: " << argv0 << " <port> " << std::endl;
    return ret;
}

using namespace ProtoTransformer;

struct Durations
{
    std::set<time_t> themselves;
    std::mutex locker;
};

int doSomething(const AnySessionHdr &, const RequestData &, AnySessionSpecific &sessionSpecific, AnyAnswerHdr &, AnswerData &, Durations *);

struct OnInterrupt
{
    struct BeforeServerStop
    {
        void operator()(int sigNum)
        { std::cout << "Actions before server will be stopped by signal "
          << sigNum << " are in progress..." << std::endl; }
    };
};

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

    typedef Server<ProtoWithSessionHdr,
                   UsePolicy<SessionSpecificIs, AnySessionSpecific>,
                   UsePolicy<InitSessionSpecificIs, InitAnySessionSpecific>,
                   UsePolicy<ServerSpaceIs, Durations>,
                   UsePolicy<SigintHandlerIs, OnInterrupt>
                  > ServerInstance;

    Durations durations;
    ServerInstance(port, doSomething, &durations);

    return 0;
}

int doSomething(
    const AnySessionHdr &sessionHdr,
    const RequestData &inBuffer,
    AnySessionSpecific &sessionSpecific,
    AnyAnswerHdr &answerHdr,
    AnswerData &outBuffer,
    Durations *durations)
{
    const char *tag = "At a session named ";
    outBuffer = AnswerData(tag, tag + strlen(tag));
    std::copy(getName(sessionHdr),
              getName(sessionHdr) + strlen(getName(sessionHdr)),
              std::back_inserter(outBuffer));
    outBuffer.push_back('\n');
    std::copy(inBuffer.begin(),
              inBuffer.end(),
              std::back_inserter(outBuffer));

    uint32_t &numOfRequestsLeft = *getNumPtr(answerHdr);
    numOfRequestsLeft = -- sessionSpecific.numOfRequestsLeft;
    if (!numOfRequestsLeft)
    {
        time_t duration = time(0) - sessionSpecific.startedAt;
        std::stringstream sessionFooter;

        {
            std::lock_guard<std::mutex> lockGuard(durations->locker);
            auto inserted = durations->themselves.insert(duration).first;
            sessionFooter << std::endl
                          << " Session duration: " << duration << " second(s); " << std::endl
                          << " Rating position: "  << std::distance(durations->themselves.begin(), inserted) + 1 << "; ";
        }

        std::string sessionFooterStr = sessionFooter.str();
        std::copy(sessionFooterStr.begin(),
                  sessionFooterStr.end(),
                  std::back_inserter(outBuffer));
    }

    return numOfRequestsLeft;
}
