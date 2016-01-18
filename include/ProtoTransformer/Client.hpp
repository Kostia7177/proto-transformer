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

#include <type_traits>
#include <string>
#include <boost/asio.hpp>
#include "../TricksAndThings/ParamPackManip/Params2Hierarchy.hpp"
#include "detail/BindArgsWithProxies.hpp"
#include "detail/AnswerCases.hpp"
#include "../TricksAndThings/EasyTraits/EasyTraits.hpp"
#include "DefaultSettingsList.hpp"

namespace ProtoTransformer
{

template<class ParamProto, class... Params>
class Client
{
    struct Cfg
        : ParamProto,
          TricksAndThings::EasyTraits<DefaultSettingsList, Int2Type<ParamProto::Domain::value + 1>, Params...>
    {
    };
    typedef typename Cfg::AnswerHdr::Itself AnswerHdr;
    typedef typename Cfg::AnswerCompletion AnswerCompletion;
    enum
    {
        answerHdrNotDefined        = std::is_same<AnswerHdr, NullType>::value,
        answerCompletionNotDefined = std::is_same<AnswerCompletion, NullType>::value
    };
    static_assert(answerHdrNotDefined || answerCompletionNotDefined,
                  "Both of answer header and answer completion function cannot be defined. Sorry so much. Fix your proto by choosing one of them and turning to NullType another. ");
    static_assert(!(answerHdrNotDefined && answerCompletionNotDefined),
                  "One (exactly one) of either answer header or answer completion function must be defined. Fix your proto. ");

    Asio::io_service ioService;
    Socket ioSocket;
    std::string serverAddr;
    int serverPort;
    Ip::tcp::endpoint endPoint;

    typedef typename Cfg::RequestData RequestData;
    typedef typename Cfg::AnswerData AnswerData;

    TricksAndThings::Params2Hierarchy
        <TricksAndThings::BindArgs,
            detail::DataHeaderWrapper<typename Cfg::RequestHdr &>,
            const RequestData &,
            detail::DataHeaderWrapper<typename Cfg::AnswerHdr *>
        > requestParams;

    enum { requestHdrIdx = 1, dataIdx, answerHdrIdx };

    typedef typename Cfg::SessionHdr SessionHdr;
    typedef typename Cfg::ClientGlobalSpace GlobalContext;

    SessionHdr sessionHdr;
    GlobalContext *globalContext;
    AnswerData answer;

    typedef typename Cfg::
                     ReadingManager::
                     template Itself<AnswerCompletion,
                                     typename Cfg::AnswerDataRepr> AnswerReadingManager;
    AnswerReadingManager readingManager;
    typename Cfg::AnswerTimeout readingTimer;

    template<class T>
    void writeSw(const T &hdr)      { write(ioSocket, Asio::buffer(&hdr, sizeof(hdr))); }
    void writeSw(const NullType &)  {}

    template<typename H>
    void setSizeSw(H &, const RequestData &);
    void setSizeSw(NullType, const RequestData &);

    typedef Int2Type<true> NoAnswerSupposed;
    typedef Int2Type<false> AnyAnswerCanBe;

    template<class F, typename H>
    void readAnswerSw(const F &, const H &, const NoAnswerSupposed &);
    template<class F>
    void readAnswerSw(const F &, NullType, const AnyAnswerCanBe &);
    template<typename H>
    void readAnswerSw(NullType, H &, const AnyAnswerCanBe &);

    public:

    template<class... Args>
    Client(const std::string &, const int, Args &&...);
    ~Client(){}

    template<typename... Args> void send(Args &&...);
    template<typename... Args> const AnswerData &request(Args &&...);
};

}
#include "detail/Client.tcc"
