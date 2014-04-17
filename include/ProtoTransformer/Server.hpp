#pragma once
/*
    Easy way to make your own application-layer protocol. Like a robot-transformer.
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

#include "detail/Session.hpp"
#include "detail/SettingSelector.hpp"
#include <boost/asio.hpp>

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
        requestHdrNotDefined        = std::is_same<typename ParamProto::RequestHdr, NullType>::value,
        requestCompletionNotDefined = std::is_same<typename ParamProto::RequestCompletion, NullType>::value
    };
    static_assert(requestHdrNotDefined || requestCompletionNotDefined,
                  "Both of request header and request completion function cannot be defined. (Hint: Choose one of them and turn to NullType another.) ");
    static_assert(!(requestHdrNotDefined && requestCompletionNotDefined),
                  "One (and strictly one) of either request header or request completion function must be defined. ");
    static_assert(ParamProto::selectorIdx == 0,
                  "First template parameter must be a proto here! ");
    static_assert(!(ParamProto::serverSendsAnswer == atLeastHdr
                    && std::is_same<typename ParamProto::AnswerHdr, NullType>::value),
                  "Server cannot return at least header of an answer when it is defined as NullType! Fix Your proto. (Hint: define an answer's header as any meaningfull OR set 'serverSendsAnswer' proto parameter to 'nothingIfNoData'. ) ");
    //
    // ...and other parameters correctness;
    NonProtoParamsChecker<Params...> paramsAreOk;

    struct Cfg
        : SettingSelector<ParamProto::selectorIdx + 1, Params...>,
          public ParamProto
    {   // merging Proto with other parameters
    };

    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor;
    typename Cfg::ThreadPool workingThreads;

    typedef typename Cfg::SessionManager::template Itself<Session<Cfg>> SessionManager;
    typedef std::shared_ptr<SessionManager> SessionManagerPtr;
    SessionManagerPtr sessionManagerPtr;

    Server(const Server &);
    Server &operator= (const Server &);

    // working body;
    template<class F> void startAccepting(Cfg, F, SessionManagerPtr);

    public:

    Server(size_t port, size_t numOfWorkers)
        : acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){}
    template<class F> Server(size_t port, F);
    ~Server(){}

    template<class F> void accept(F);
    void stop(){ ioService.stop(); }
};

}
#include "detail/Server.tcc"
