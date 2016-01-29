#pragma once

#include "SessionContext.hpp"
#include "RequestContext.hpp"
#include "SessionAdministration.hpp"

namespace ProtoTransformer
{

typedef std::shared_ptr<Socket> SocketPtr;

template<class Cfg>
class Session
    : public std::enable_shared_from_this<Session<Cfg>>,
      public Asio::coroutine
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
    bool goOn;

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
            : exitDetector(s), itself(f){}
    };

    template<class F>
    using PayloadPtr = std::shared_ptr<Payload<F>>;

    enum Phase
    {
        unspecified,
        readingSessionHdr,
        readingHdr,
        readingData,
        writingHdr,
        writingData
    };

    using ErrorMessages = std::map<Phase, std::string>;
    static  ErrorMessages errorMessage;

    template<class F>
    struct Tappet
    {
        const PayloadPtr<F> payloadPtr;

        private:
        Phase phase;
        public:

        Tappet(F f, Session *s)
            : payloadPtr(std::make_shared<Payload<F>>(f, s->shared_from_this())),
              phase(unspecified)
        { payloadPtr->exitDetector.getSession().goOn = true; }

        void operator()(
            Sys::error_code errorCode = Sys::error_code(),
            size_t = 0)
        {
            Session &session = payloadPtr->exitDetector.getSession();

            if (session.administration.exitManager.sessionWasRemoved())
            {
                session.goOn = false;
            }

            if (errorCode)
            {
                session.goOn = false;
                typename Cfg::Logger &logger = session.logger;
                logger(logger.errorOccured(), "%s '%s';", session.errorMessage[phase].c_str(),errorCode.message().c_str());
            }
            kickWorkflow();
        }

        void kickWorkflow()
        { payloadPtr->exitDetector.getSession().workflow(*this); }

        const Tappet &atPhase(Phase p){ return phase = p, *this; }
    };

    // here is a working body of a session;
    template<class F> void workflow(Tappet<F>);

    // the following are phases of a working body.
    // first 1 or 2 parameters of each 'Sw'-marked function
    // provide such a compile-time overload-based Switch
    // between different possible configuration variants.
    //
    // opening a new session
    template<typename SessionHdr,
             class F> void readSessionHdrSw(const SessionHdr &, Tappet<F>);
    template<class F> void readSessionHdrSw(const NullType &, Tappet<F>);
    //
    // request processing phases (*)
    template<class RequestCompletion,
             class F> void readRequestHdrSw(const RequestCompletion &, Tappet<F>);
    template<class F> void readRequestHdrSw(const NullType &, Tappet<F>);
    template<class RequestCompletion,
             class F> void readRequestDataSw(const RequestCompletion &, Tappet<F>);
    template<class F> void readRequestDataSw(const NullType &, Tappet<F>);
    //
    template<class F> void processRequest(PayloadPtr<F>);
    //
    template<typename AnswerHdr,
             class F> void writeAnswerHdrSw(const AnswerHdr &, const NoAnswerAtAll &, Tappet<F>);
    template<typename AnswerHdr,
             class F> void writeAnswerHdrSw(const AnswerHdr &, const AtLeastHeader &, Tappet<F>);
    template<typename AnswerHdr,
             class F> void writeAnswerHdrSw(const AnswerHdr &, const NothingIfNoData &, Tappet<F>);
    template<class F> void writeAnswerHdrSw(const NullType &, const NothingIfNoData &, Tappet<F>);
    //
    template<class AnswerMode,
             class F> void writeAnswerDataSw(const AnswerMode &, Tappet<F>);
    template<class F> void writeAnswerDataSw(const NoAnswerAtAll &, Tappet<F>);
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

    template<class F> void run(F payload)
    {workflow(Tappet<F>(payload, this)); }

    typename Administration::ExitManager &getManagerReference() { return administration.exitManager; }
};

template<class Cfg>
typename Session<Cfg>::ErrorMessages Session<Cfg>::errorMessage
{
    { Session<Cfg>::unspecified, "Anything have been failed" },
    { Session<Cfg>::readingSessionHdr, "Cannot read session header:" },
    { Session<Cfg>::readingHdr, "Cannot read request header" },
    { Session<Cfg>::readingData, "Cannot read request data" },
    { Session<Cfg>::writingHdr, "Cannot write answer header" },
    { Session<Cfg>::writingData, "Cannot write answer data" }
};

}
#include "Session.tcc"
