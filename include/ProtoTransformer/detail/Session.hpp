#pragma once

#include "AnswerCases.hpp"
#include <boost/asio.hpp>

namespace ProtoTransformer
{

typedef boost::asio::ip::tcp::socket Socket;
typedef std::shared_ptr<Socket> SocketPtr;

template<class Cfg>
struct TaskData
{   // a component of a 'Session' class (see below);
    // data that is changing with every request
    // and is to be private of a request processing;
    // (privacy is actual with parallel request
    // processing, i.e. with parallelRequestsPerSession != 1
    // and SessionThreadPool != NullType);
    //
    typename Cfg::RequestHdr requestHdr;            // -- request description;
                                                    //    in main case contains
                                                    //    a simple size of a request;
                                                    //    if it so, not will be
                                                    //    passed to a payload code;
    typedef std::vector<typename Cfg::RequestDataRepr> Request;
    Request inDataBuffer;                           // -- request data itself;
    typename Cfg::AnswerHdr answerHdr;              // -- the same as requestHdr, but
                                                    //    for answer;
    typedef std::vector<typename Cfg::AnswerDataRepr> Answer;
    Answer outDataBuffer;                           // -- guess what :)

    template<int any>
    void Swap(TaskData &data, const Int2Type<any> &)
    {   // providing a private copy of a request and answer
        // data to pass it to a payload (used within parallel
        // request processing);
        requestHdr = data.requestHdr;               // in most cases is a something
                                                    // simple, so - simple copy it;
        std::swap(inDataBuffer, data.inDataBuffer); // is a vector - it's better to
                                                    // swap it;
        answerHdr = data.answerHdr;                 // and the same for answer header;
        std::swap(outDataBuffer, data.outDataBuffer);   // ...and data;
    }

    enum { serverSendsAnswer = Cfg::serverSendsAnswer };
    void Swap(TaskData &data, const Int2Type<never> &)
    {   // omit answer buffers copying/swapping
        // if they are not used;
        requestHdr = data.requestHdr;
        std::swap(inDataBuffer, data.inDataBuffer);
    }
};

template<class Cfg>
class Session
    : public std::enable_shared_from_this<Session<Cfg>>
{   // every user's connection will be served by
    // an instance of this 'Session' class;
    //
    // different kinds of a session data:
    //
    typename Cfg::SessionHdr sessionHdr;            // -- session invariant, sent by
                                                    //    client after the connection
                                                    //    have been established;
    typename Cfg::SessionSpecific sessionSpecific;  // -- session data (alternate to
                                                    //    an invariant sessionHdr, is
                                                    //    a variable data); available
                                                    //    for each request;
                                                    //    such a request-static data;
                                                    //
    typename Cfg::InitSessionSpecific initSessionSpecific;  // user-defined function,
                                                            // that initialises session-
                                                            // specific (may be by
                                                            // session header);
    typedef TaskData<Cfg> TaskBuffers;
    TaskBuffers taskBuffers;                        // -- aggregated request and answer
                                                    //    headers and data buffers;

    SocketPtr ioSocketPtr;

    // a thing that controls the reading,
    // if no the request size is known...
    typedef typename Cfg::
                     ReadingManager::
                     template Itself<typename Cfg::RequestCompletion,
                                     typename Cfg::RequestDataRepr> RequestReadingManager;
    RequestReadingManager readingManager;
    // ...but it's completion can be recognized
    // while reading the request body - the following
    // is a user-defined code, that it provides;
    typename RequestReadingManager::Completion requestCompletion;

    typedef typename Cfg::SessionManager::template Reference<Session> ManagerReference;
    ManagerReference manager;
    typename Cfg::TaskManager taskManager;

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

    public:

    Session(Cfg cfg, SocketPtr arg)
        : ioSocketPtr(arg),
          readingManager(taskBuffers.inDataBuffer),
          taskManager(getNumOfThreads(Cfg::numOfRequestsPerSession)){}
    ~Session(){}

    template<class F> void run(F);

    ManagerReference &getManagerReference() { return manager; }
};

}
#include "Session.tcc"
