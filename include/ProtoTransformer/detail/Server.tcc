/*
    Easy way to make your own application-layer protocol. Like a robot-transformer.
    Copyright (C) 2014  Konstantin U. Zozoulia

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

namespace ProtoTransformer
{

template<class ParamProto, class... Params>
template<class F>
Server<ParamProto, Params...>::Server(
    size_t port,
    F payloadCode)
    : acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      workingThreads(SettingSelector<1, Params...>::getNumOfWorkers())
{
    accept(payloadCode);
}

template<class ParamProto, class... Params>
template<class F>
void Server<ParamProto, Params...>::accept(F payload)
{
    sessionManagerPtr = SessionManagerPtr(new SessionManager);
    startAccepting(Cfg(), payload, sessionManagerPtr);
    for (size_t idx = 0; idx < workingThreads.size(); workingThreads.schedule([&] { ioService.run(); }), ++ idx);
    workingThreads.wait();
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
                          [=] (const boost::system::error_code &errorCode)
                          {
                              if (!errorCode)
                              {
                                  sessionManagerPtr->startSession(std::make_shared<Session<Cfg>>(cfg, newSocketPtr), payload);
                                  startAccepting(cfg, payload, sessionManagerPtr);
                              }
                              else { stop(); }
                          });
}

}
