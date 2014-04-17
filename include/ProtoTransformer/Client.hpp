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

#include <type_traits>
#include <string>
#include <boost/asio.hpp>
#include "detail/AnswerCases.hpp"

namespace ProtoTransformer
{

enum AnswerAwaiting { oneWayRequest, answerSupposed };

template<class Cfg>
class Client
{
    enum
    {
        answerHdrNotDefined        = std::is_same<typename Cfg::AnswerHdr, NullType>::value,
        answerCompletionNotDefined = std::is_same<typename Cfg::AnswerCompletion, NullType>::value
    };
    static_assert(answerHdrNotDefined || answerCompletionNotDefined,
                  "Both of answer header and answer completion function cannot be defined. Sorry so much. Fix your proto by choosing one of them and turning to NullType another. ");
    static_assert(!(answerHdrNotDefined && answerCompletionNotDefined),
                  "One (exactly one) of either answer header or answer completion function must be defined. Fix your proto. ");

    boost::asio::io_service ioService;
    boost::asio::ip::tcp::socket ioSocket;
    boost::asio::ip::tcp::endpoint endPoint;
    typedef typename Cfg::RequestData RequestData;
    typedef typename Cfg::AnswerData AnswerData;
    AnswerData answer;
    typedef typename Cfg::
                     ReadingManager::
                     template Itself<typename Cfg::AnswerCompletion,
                                     typename Cfg::AnswerDataRepr> AnswerReadingManager;
    AnswerReadingManager readingManager;
    typedef typename Cfg::SessionHdr SessionHdr;
    SessionHdr sessionHdr;

    template<class T>
    void writeSw(const T &hdr)      { write(ioSocket, boost::asio::buffer(&hdr, sizeof(hdr))); }
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

    Client(const std::string &, int, SessionHdr = SessionHdr());
    ~Client(){}

    const AnswerData &request(typename Cfg::RequestHdr &, const RequestData &, typename Cfg::AnswerHdr *, AnswerAwaiting = answerSupposed);
    const AnswerData &request(typename Cfg::RequestHdr &, const RequestData &, AnswerAwaiting = answerSupposed);
    const AnswerData &request(const RequestData &, typename Cfg::AnswerHdr *, AnswerAwaiting = answerSupposed);
    const AnswerData &request(const RequestData &, AnswerAwaiting = answerSupposed);
};

}
#include "detail/Client.tcc"
