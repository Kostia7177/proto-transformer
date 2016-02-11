#pragma once

#include "SessionContext.hpp"
#include "RequestContext.hpp"
#include "SessionAdministration.hpp"

namespace ProtoTransformer
{

typedef std::shared_ptr<Socket> SocketPtr;

template<class Cfg>
class Session
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

    // the following is working body phases represented in
    // overloads. first 1 or 2 parameters of each 'Sw'-marked
    // function provide such a compile-time overload-based
    // Switch between different possible configuration variants.
    //
    // opening a new session
    template<typename SessionHdr,
             class W> void readSessionHdrSw(const SessionHdr &, W &);
    template<class W> void readSessionHdrSw(const NullType &, W &);
    //
    template<class InitSessionSpecific>
    void initSessionSpecificSw(const InitSessionSpecific &);
    void initSessionSpecificSw(NullType) {}
    //
    // overloads of request processing phases
    template<class RequestCompletion,
             class W> void readRequestHdrSw(const RequestCompletion &, W &);
    template<class W> void readRequestHdrSw(const NullType &, W &);
    //
    template<class RequestCompletion,
             class W> void readRequestDataSw(const RequestCompletion &, const W &);
    template<class W> void readRequestDataSw(const NullType &, const W &);
    //
    //
    template<typename AnswerHdr,
             class W> void writeAnswerHdrSw(const AnswerHdr &, const NoAnswerAtAll &, W &);
    template<typename AnswerHdr,
             class W> void writeAnswerHdrSw(const AnswerHdr &, const AtLeastHeader &, W &);
    template<typename AnswerHdr,
             class W> void writeAnswerHdrSw(const AnswerHdr &, const NothingIfNoData &, W &);
    template<class W> void writeAnswerHdrSw(const NullType &, const NothingIfNoData &, W &);

    void setTimer();
    void cancelTimer() { readingTimeout.cancel(); }

    public:

    Session(
        Asio::io_service &ioService,
        Socket &&socketArg,
        ServerSpace *serverSpaceArg,
        typename Cfg::TaskManager &managerRef)
        : serverSpace(serverSpaceArg),
          administration(requestContext.inDataBuffer, managerRef),
          ioSocket(std::move(socketArg)),
          readingTimeout(ioService)
    { logger(logger.debug(), "New session %#zx started; ", this);}

    ~Session() { logger(logger.debug(), "Closing the session %#zx; ", this); }

    typename Administration::ExitManager &getManagerReference()
    { return administration.exitManager; }

    // starting a new session...
    template<class W> void run(W &workflow)
    { readSessionHdrSw(sessionContext.sessionHdr, workflow[W::readingSessionHdr]); }
    //
    void initSessionSpecific()
    { initSessionSpecificSw(sessionContext.initSessionSpecific); }
    // ...and repeat the request processing phases (*)
    template<class W>
    void readRequestHdr(W &workflow)
    { readRequestHdrSw(requestCompletion, workflow[W::readingHdr]); }
    //
    template<class W>
    void readRequestData(W &workflow)
    { readRequestDataSw(requestCompletion, workflow[W::readingData]); }
    //
    template<class PayloadPtr>
    void processRequest(PayloadPtr);
    //
    template<class W, class AnswerMode>
    void writeAnswerHdr(const AnswerMode &mode, W &workflow)
    { writeAnswerHdrSw(requestContext.answerHdr, mode, workflow); }
    //
    template<class AnswerMode,
             class W> void writeAnswerDataSw(const AnswerMode &, W &);
    template<class W> void writeAnswerDataSw(const NoAnswerAtAll &, W &);
    //
    // ...and so on - go to a new request (*);
};

}
#include "Session.tcc"
