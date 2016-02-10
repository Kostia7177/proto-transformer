#pragma once
/*
    Easy way to make your own application-layer protocol. Just play with it like with a robot-transformer.
    Copyright (C) 2014  Konstantin U. Zozoulia

    candid.71 -at- mail -dot- ru

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "detail/Session/Session.hpp"
#include "DefaultSettingsList.hpp"

namespace ProtoTransformer
{

template<class ParamProto, class... Params>
class Server
{   // what do You think the class 'Server' is?..
    // (hint: may be... a client???) ;)
    //
    // listens for new connections and creates new
    // sessions; checks the configuration for a
    // consistence; keeps shared-pointer to a
    // session manager;
    //
    // ...first, let's check Proto consistence...
    enum
    {
        requestHdrNotDefined        = std::is_same<typename ParamProto::RequestHdr::Itself, NullType>::value,
        requestCompletionNotDefined = std::is_same<typename ParamProto::RequestCompletion, NullType>::value
    };
    static_assert(requestHdrNotDefined || requestCompletionNotDefined,
                  "Both of request header and request completion function cannot be "
                  "defined. (Hint: Choose one of them and turn to NullType another.) ");
    static_assert(!(requestHdrNotDefined && requestCompletionNotDefined),
                  "One (and strictly one) of either request header or request "
                  "completion function must be defined. ");
    static_assert(ParamProto::Domain::value == 0,
                  "First template parameter must be a proto here! ");
    static_assert(!(ParamProto::serverSendsAnswer == atLeastHdr
                    && std::is_same<typename ParamProto::AnswerHdr, NullType>::value),
                  "Server cannot return at least header of an answer when it is defined "
                  "as NullType! Fix Your proto. (Hint: define an answer's header as any "
                  "meaningfull OR set 'serverSendsAnswer' proto parameter to 'nothingIfNoData'. ) ");
    //
    // ...and other parameters correctness;
    NonProtoParamsChecker<Params...> paramsAreOk;

    struct Cfg
        : TricksAndThings::EasyTraits<DefaultSettingsList, Int2Type<ParamProto::Domain::value + 1>, Params...>,
          public ParamProto
    {   // merging Proto with other parameters
    };
    static_assert(std::is_same<typename Cfg::TaskManager::Itself, NullType>::value
                  || (!std::is_same<typename Cfg::TaskManager::Itself, NullType>::value
                      && Cfg::serverSendsAnswer == never),
                  "Parallel requests processing within a session is available with "
                  "no-answer proto only! ");

    Asio::io_service ioService;
    Ip::tcp::acceptor acceptor;
    typename Cfg::ServerThreadPool workingThreads;
    typedef typename Cfg::ServerSpace ServerSpace;
    ServerSpace *serverSpace;

    struct WorkflowIfc
    {
        virtual ~WorkflowIfc(){}

        enum Phase
        {
            unspecified,
            accepting,
            readingSessionHdr,
            readingHdr,
            readingData,
            writingHdr,
            writingData
        };
    };

    using ErrorMessages = std::map<typename WorkflowIfc::Phase, std::string>;
    static ErrorMessages errorMessage;

    typedef typename Cfg::SessionManager::template Itself<Session<Cfg>> SessionManager;
    typedef std::shared_ptr<SessionManager> SessionManagerPtr;
    SessionManagerPtr sessionManagerPtr;

    template<class F>
    class Workflow
        : public WorkflowIfc,
          public Asio::coroutine
    {
        Server *server;
        SocketPtr newSocketPtr;

        typedef typename Cfg::SessionManager::template ExitDetector<Session<Cfg>> ExitDetector;
        // payload container plus session keeper
        // (shared-pointer-based exit detector);
        // will be passed by copying from one phase to another
        // until one of the phases will return (due to error
        // or at the exit-condition, which is 0 returned by
        // payload)
        // 'F' is a payload code that will be called inside
        // 'processRequest' (we cannot include it into 'Cfg')
        struct Payload
        {
            ExitDetector exitDetector;
            F itself;
            Payload(F f, std::shared_ptr<Session<Cfg>> s)
                : exitDetector(s), itself(f){}
        };

        std::shared_ptr<Session<Cfg>> newSession;
        Session<Cfg> *session;

        std::shared_ptr<Payload> payloadPtr;
        std::shared_ptr<F> payloadOrig;

        SessionManagerPtr sessionManagerPtr;
        typename WorkflowIfc::Phase phase;
        typename Cfg::Logger logger;

        typedef Sys::error_code SysErrorCode;

        public:

        Workflow(Server *s, F f, SessionManagerPtr p)
            : server(s),
              session(0),
              payloadOrig(new F(f)),
              sessionManagerPtr(p),
              phase(WorkflowIfc::unspecified)
        {
            (*this)();
        }

        void operator()(SysErrorCode = SysErrorCode(), size_t = 0);

        Workflow &operator[](typename WorkflowIfc::Phase p)
        { return phase = p, *this; }
    };
    std::unique_ptr<WorkflowIfc> workflow;
    typename Cfg::TaskManager taskManager;

    Asio::signal_set sigHandler;
    Server(const Server &);
    Server &operator= (const Server &);

    void setupSigHandler(NullType){}
    template<class H>
    void setupSigHandler(const H &);

    public:

    Server(size_t port, size_t numOfWorkers, ServerSpace *inServerSpace = 0)
        : acceptor(ioService, Ip::tcp::endpoint(Ip::tcp::v4(), port)),
          serverSpace(inServerSpace),
          taskManager(getNumOfThreads(Cfg::numOfRequestsPerSession)),
          sigHandler(ioService){}
    template<class F> Server(size_t port, F, ServerSpace * = 0);
    ~Server(){}

    template<class F> void accept(F);
    void stop(){ ioService.stop(); }
};

template<class ParamProto, class... Params>
typename Server<ParamProto, Params...>::ErrorMessages Server<ParamProto, Params...>::errorMessage
{
    { Server<ParamProto, Params...>::WorkflowIfc::unspecified, "Anything have been failed" },
    { Server<ParamProto, Params...>::WorkflowIfc::readingSessionHdr, "Cannot read session header:" },
    { Server<ParamProto, Params...>::WorkflowIfc::readingHdr, "Cannot read request header" },
    { Server<ParamProto, Params...>::WorkflowIfc::readingData, "Cannot read request data" },
    { Server<ParamProto, Params...>::WorkflowIfc::writingHdr, "Cannot write answer header" },
    { Server<ParamProto, Params...>::WorkflowIfc::writingData, "Cannot write answer data" }
};

}
#include "detail/Server.tcc"
