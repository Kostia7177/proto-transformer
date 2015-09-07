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
#include"signalHandlerSetupSwitchers.hpp"

namespace ProtoTransformer
{

template<class Proto, class... Params>
template<class Handler>
void Server<Proto, Params...>::setupSigHandler(const Handler &handler)
{
    sigHandler.add(SIGINT);
    sigHandler.add(SIGTERM);
    sigHandler.async_wait([=](const Sys::error_code &errorCode, int sigNum)
                          {
                            if (!errorCode)
                            {
                                beforeStopSw(Int2Type<sizeof(withBeforeStopActions<Handler>(0)) == sizeof(One)>(), handler, sigNum);
                                stop();
                            }
                          });
}

template<class Proto, class... Params>
template<class F>
void Server<Proto, Params...>::startAccepting(
    Cfg cfg,
    F payload,
    SessionManagerPtr sessionManagerPtr)
{
    SocketPtr newSocketPtr(new Socket(ioService));
    acceptor.async_accept(*newSocketPtr,
                          [=] (const Sys::error_code &errorCode)
                          {
                            if (!errorCode)
                            {
                                std::shared_ptr<Session<Cfg>> newSession = std::make_shared<Session<Cfg>>(cfg, ioService, newSocketPtr, serverSpace);
                                sessionManagerPtr->startSession(newSession, payload);
                                logger.itself(logger.itself.debug(), "New session %zx started; ", newSession.get());
                                startAccepting(cfg, payload, sessionManagerPtr);
                            }
                            else
                            {
                                logger.itself(logger.itself.errorOccured(), "Stopping server due to error '%s'; ", errorCode.message().c_str());
                                stop();
                            }
                          });
}

template<class ParamProto, class... Params>
template<class F>
Server<ParamProto, Params...>::Server(
    size_t port,
    F payloadCode,
    ServerSpace *inServerSpace)
    : acceptor(ioService, Ip::tcp::endpoint(Ip::tcp::v4(), port)),
      workingThreads(getNumOfThreads(Cfg::numOfWorkers)),
      serverSpace(inServerSpace),
      sigHandler(ioService)
{
    accept(payloadCode);
}

template<class ParamProto, class... Params>
template<class F>
void Server<ParamProto, Params...>::accept(
    F payload)
{
    sessionManagerPtr = SessionManagerPtr(new SessionManager);
    startAccepting(Cfg(), payload, sessionManagerPtr);
    for (size_t idx = 0; idx < workingThreads.size(); workingThreads.schedule([&] { ioService.run(); }), ++ idx);
    setupSigHandler(typename Cfg::SigintHandler());
    workingThreads.wait();
}

}
