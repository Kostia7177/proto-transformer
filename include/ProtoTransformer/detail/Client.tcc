/*
    Easy way to make your own application-layer protocol. Just play with it like with a robot-transformer.
    Copyright (C) 2014  Konstantin U. Zozoulia

    candid -at- mail -dot- ru

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

template<class Cfg>
template<typename H>
void Client<Cfg>::setSizeSw(
    H &hdr, // there is any request header specified,
    const RequestData &data)
{           // so we can set the request size into it;
    Cfg::RequestHdr::setSize2(data.size() * sizeof(typename Cfg::RequestDataRepr), hdr);
}

template<class Cfg>
void Client<Cfg>::setSizeSw(
    NullType,   // request header is not specified;
    const RequestData &)
{
}

template<class Cfg>
template<class F, typename H>
void Client<Cfg>::readAnswerSw(const F &, const H &, const NoAnswerSupposed &)
{
}

template<class Cfg>
template<class F>
void Client<Cfg>::readAnswerSw(
    const F &,              // completion function specified
    NullType,               // instead of answer header;
    const AnyAnswerCanBe &)
{
    readingManager.get(ioSocket, sessionHdr);
}

template<class Cfg>
template<typename H>
void Client<Cfg>::readAnswerSw(
    NullType,               // no completion function specified,
    H &answerHdr,           // but some answer header expected;
    const AnyAnswerCanBe &)
{
    read(ioSocket, boost::asio::buffer(&answerHdr, sizeof(answerHdr)));
    if (uint32_t sizeOfAnswer = Cfg::AnswerHdr::getSize(answerHdr) / sizeof(typename Cfg::AnswerDataRepr))
    {
        answer.resize(sizeOfAnswer);
        read(ioSocket, boost::asio::buffer(answer));
    }
}

template<class Cfg>
Client<Cfg>::Client(
    const std::string &serverAddr,
    int serverPort,
    SessionHdr hdr)
    : ioSocket(ioService),
      endPoint(boost::asio::ip::address::from_string(serverAddr), serverPort),
      readingManager(answer),
      sessionHdr(hdr)
{
    ioSocket.connect(endPoint);
    writeSw(sessionHdr);
}

template<class Cfg>
const typename Client<Cfg>::AnswerData &Client<Cfg>::request(
    RequestHdr &requestHdr,
    const RequestData &data,
    AnswerHdr *answerHdr,
    AnswerAwaiting answerAwaiting)
{
    setSizeSw(requestHdr, data);
    writeSw(requestHdr);
    write(ioSocket, boost::asio::buffer(data));
    if (answerAwaiting == answerSupposed)
    {
        readAnswerSw(typename AnswerReadingManager::Completion(), *answerHdr, Int2Type<Cfg::serverSendsAnswer == never>());
    }
    return answer;
}

template<class Cfg>
const typename Client<Cfg>::AnswerData &Client<Cfg>::request(
    RequestHdr &requestHdr,
    const RequestData &data,
    AnswerAwaiting answerAwaiting)
{
    AnswerHdr answerHdr;
    request(requestHdr, data, &answerHdr, answerAwaiting);
    return answer;
}

template<class Cfg>
const typename Client<Cfg>::AnswerData &Client<Cfg>::request(
    const RequestData &data,
    AnswerHdr *answerHdr,
    AnswerAwaiting answerAwaiting)
{
    RequestHdr requestHdr;
    request(requestHdr, data, answerHdr, answerAwaiting);
    return answer;
}

template<class Cfg>
const typename Client<Cfg>::AnswerData &Client<Cfg>::request(
    const RequestData &data,
    AnswerAwaiting answerAwaiting)
{
    RequestHdr requestHdr;
    AnswerHdr answerHdr;
    request(requestHdr, data, &answerHdr, answerAwaiting);
    return answer;
}

}
