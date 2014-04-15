namespace ProtoTransformer
{

template<class Cfg>
template<typename H>
void Client<Cfg>::setSizeSw(
    H &hdr,
    const RequestData &data)
{
    typename Cfg::SetSizeOfRequest2Hdr setSizeOfRequest2Hdr;
    setSizeOfRequest2Hdr(data.size() * sizeof(typename Cfg::RequestDataRepr), hdr);
}

template<class Cfg>
void Client<Cfg>::setSizeSw(
    NullType,
    const RequestData &)
{
}

template<class Cfg>
template<typename H>
void Client<Cfg>::readAnswerSw(
    NullType,
    H &answerHdr,
    const AnyAnswerCanBe &)
{
    read(ioSocket, boost::asio::buffer(&answerHdr, sizeof(answerHdr)));
    typename Cfg::GetSizeOfAnswerFromHdr getSizeOfAnswerFromHdr;
    if (uint32_t sizeOfAnswer = getSizeOfAnswerFromHdr(answerHdr) / sizeof(typename Cfg::AnswerDataRepr))
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
    typename Cfg::RequestHdr &requestHdr,
    const RequestData &data,
    typename Cfg::AnswerHdr *answerHdr,
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
    typename Cfg::RequestHdr &requestHdr,
    const RequestData &data,
    AnswerAwaiting answerAwaiting)
{
    typename Cfg::AnswerHdr answerHdr;
    request(requestHdr, data, &answerHdr, answerAwaiting);
    return answer;
}

template<class Cfg>
const typename Client<Cfg>::AnswerData &Client<Cfg>::request(
    const RequestData &data,
    typename Cfg::AnswerHdr *answerHdr,
    AnswerAwaiting answerAwaiting)
{
    typename Cfg::RequestHdr requestHdr;
    request(requestHdr, data, answerHdr, answerAwaiting);
    return answer;
}

template<class Cfg>
const typename Client<Cfg>::AnswerData &Client<Cfg>::request(
    const RequestData &data,
    AnswerAwaiting answerAwaiting)
{
    typename Cfg::RequestHdr requestHdr;
    typename Cfg::AnswerHdr answerHdr;
    request(requestHdr, data, &answerHdr, answerAwaiting);
    return answer;
}

}
