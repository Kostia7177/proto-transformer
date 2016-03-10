#pragma once

#include "SessionContext.hpp"
#include "RequestContext.hpp"
#include "SessionAdministration.hpp"
#include<boost/asio/spawn.hpp>

namespace ProtoTransformer
{

typedef std::shared_ptr<Socket> SocketPtr;
typedef Asio::yield_context YieldContext;

template<class Cfg>
class Session
    : public std::enable_shared_from_this<Session<Cfg>>
{   // every user's connection will be served by
    // an instance of this 'Session' class;
    //
    // different kinds of a session data:
    //
    RequestContext<Cfg> requestContext;        // -- aggregated request and answer
                                               //    headers and data buffers;
    SessionContext<Cfg> sessionContext;
    typedef typename Cfg::ServerSpace ServerSpace;
    ServerSpace *serverSpace;

    typedef SessionAdministration<Cfg, Session<Cfg>> Administration;
    Administration administration;
    typename Administration::RequestCompletion requestCompletion;

    typename Cfg::Logger logger;

    Socket ioSocket;

    typename Cfg::RequestTimeout readingTimeout;

    Session(const Session &);
    Session &operator= (const Session &);

    typedef typename Cfg::SessionManager::template ExitDetector<Session> ExitDetector;
    // payload container plus session keeper
    // (shared-pointer-based exit detector);
    // will be passed by copying from one phase to another
    // until one of the phases will return (due to error
    // or at the exit-condition, which is 0 returned by
    // payload)
    // 'F' is a payload code that will be called inside
    // 'processRequest' (we cannot include it into 'Cfg')
    template<class F>
    struct Payload
    {
        ExitDetector exitDetector;
        F itself;
        Payload(F f, std::shared_ptr<Session> s)
            : exitDetector(s), itself(f) {}
    };

    template<class F>
    using PayloadPtr = std::shared_ptr<Payload<F>>;

    enum Phase
    {
        readingSessionHdr,
        readingRequestHdr,
        readingRequestData,
        writingAnswerHdr,
        writingAnswerData
    };
    Phase phase;

    using ErrorMessages = std::map<Phase, std::string>;
    static ErrorMessages errorMessages;

    struct SessionWasRemoved
    : std::exception {};

    void throwIfRemoved();

    // the following is a working body of a session.
    // first 1 or 2 parameters of each 'Sw'-marked function
    // provide such a compile-time overload-based Switch
    // between different possible configuration variants.
    //
    // opening a new session
    template<typename SessionHdr> void readSessionHdrSw(SessionHdr &, const YieldContext &);
    void readSessionHdrSw(NullType &, const YieldContext &);
    //
    // request processing phases (*)
    template<class RequestHdr> void readRequestHdrSw(RequestHdr &, const YieldContext &);
    void readRequestHdrSw(NullType &, const YieldContext &);
    template<class RequestCompletion> void readRequestDataSw(const RequestCompletion &, const YieldContext &);
    void readRequestDataSw(const NullType &, const YieldContext &);
    //
    template<class F> void processRequest(PayloadPtr<F>);
    //
    template<typename AnswerHdr> void writeAnswerHdrSw(AnswerHdr &, const NoAnswerAtAll &, const YieldContext &);
    template<typename AnswerHdr> void writeAnswerHdrSw(AnswerHdr &, const AtLeastHeader &, const YieldContext &);
    template<typename AnswerHdr> void writeAnswerHdrSw(AnswerHdr &, const NothingIfNoData &, const YieldContext &);
    void writeAnswerHdrSw(NullType &, const NothingIfNoData &, const YieldContext &);
    //
    template<class Condition> void writeAnswerData(const Condition &, const YieldContext &);
    void writeAnswerData(const NoAnswerAtAll &, const YieldContext &);
    // ...and so on - go to a new request (*);

    template<class InitSessionSpecific>
    void initSessionSpecificSw(const InitSessionSpecific &);
    void initSessionSpecificSw(NullType) {}

    void setTimer();
    void cancelTimer() { readingTimeout.cancel(); }

    public:

    Session(
        Asio::io_service &ioService,
        SocketPtr socketPtrArg,
        ServerSpace *serverSpaceArg,
        typename Cfg::TaskManager &managerRef)
        : serverSpace(serverSpaceArg),
          administration(requestContext.inDataBuffer, managerRef),
          ioSocket(std::move(*socketPtrArg)),
          readingTimeout(ioService)
    { logger(logger.debug(), "New session %#zx started; ", this); }

    ~Session() { logger(logger.debug(), "Closing the session %#zx; ", this); }

    template<class F> void run(F);

    typename Administration::ExitManager &getManagerReference() { return administration.exitManager; }
};

template<class Cfg>
typename Session<Cfg>::ErrorMessages Session<Cfg>::errorMessages
{
    { readingSessionHdr, "Cannot read session header" },
    { readingRequestHdr, "Cannot read request header" },
    { readingRequestData, "Cannot read request data" },
    { writingAnswerHdr, "Cannot write answer header" },
    { writingAnswerData, "Cannot write answer data" }
};

}
#include "Session.tcc"
