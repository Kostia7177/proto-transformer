#pragma once

#include "SessionContext.hpp"
#include "RequestContext.hpp"
#include "SessionAdministration.hpp"

namespace ProtoTransformer
{

typedef std::shared_ptr<Socket> SocketPtr;

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

    SocketPtr ioSocketPtr;

    typename Cfg::RequestTimeout readingTimeout;

    Session(const Session &);
    Session &operator= (const Session &);

    // session keeper; shared-pointer-based exit detector.
    // will be passed by copying from one phase to another
    // until one of the phases will return (due to error
    // or at the exit-condition, which is 0 returned by
    // payload)
    typedef std::shared_ptr<typename Cfg::SessionManager::template ExitDetector<Session>> ExitDetectorPtr;

    // the following is a working body of a session.
    // first 1 or 2 parameters of each 'Sw'-marked function
    // provide such a compile-time overload-based Switch
    // between different possible configuration variants.
    // 'F' is a payload code that will be called inside
    // 'processRequest' (we cannot include it into 'Cfg')
    //
    // opening a new session
    template<typename SessionHdr,
             class F> void runSw(const SessionHdr &, F, ExitDetectorPtr);
    template<class F> void runSw(const NullType &, F, ExitDetectorPtr);
    //
    // request processing phases (*)
    template<class RequestCompletion,
             class F> void readRequestSw(const RequestCompletion &, F, ExitDetectorPtr);
    template<class F> void readRequestSw(const NullType &, F, ExitDetectorPtr);
    //
    template<class F> void processRequest(F, ExitDetectorPtr);
    //
    template<typename AnswerHdr,
             class F> void writeAnswerSw(const AnswerHdr &, const NoAnswerAtAll &, F, ExitDetectorPtr);
    template<typename AnswerHdr,
             class F> void writeAnswerSw(const AnswerHdr &, const AtLeastHeader &, F, ExitDetectorPtr);
    template<typename AnswerHdr,
             class F> void writeAnswerSw(const AnswerHdr &, const NothingIfNoData &, F, ExitDetectorPtr);
    template<class F> void writeAnswerSw(const NullType &, const NothingIfNoData &, F, ExitDetectorPtr);
    //
    template<class F> void writeAnswerData(F, ExitDetectorPtr);
    // ...and so on - go to a new request (*);

    template<class InitSessionSpecific>
    void initSessionSpecificSw(const InitSessionSpecific &);
    void initSessionSpecificSw(NullType){}
    void setTimer();
    void cancelTimer(){ readingTimeout.cancel(); }

    public:

    Session(
        Cfg cfg,
        Asio::io_service &ioService,
        SocketPtr socketPtrArg,
        ServerSpace *serverSpaceArg,
        typename Cfg::TaskManager &managerRef)
        : serverSpace(serverSpaceArg),
          administration(requestContext.inDataBuffer, managerRef),
          ioSocketPtr(socketPtrArg),
          readingTimeout(ioService){}
    ~Session(){ logger(logger.debug(), "Closing the session %#zx; ", this); }

    template<class F> void run(F);

    typename Administration::ExitManager &getManagerReference() { return administration.exitManager; }
};

}
#include "Session.tcc"
