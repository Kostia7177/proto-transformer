#include "filteringAdapter.hpp"
#include "PureHdr.hpp"

namespace ProtoTransformer
{

template<class Cfg>
template<class F>
void Session<Cfg>::run(F payload)
{
    typedef typename Cfg::SessionManager::template ExitDetector<Session> ExitDetector;
    runSw(sessionHdr, payload, std::shared_ptr<ExitDetector>(new ExitDetector(this->shared_from_this())));
}

template<class Cfg>
template<typename SessionHdr, class F>
void Session<Cfg>::runSw(
    const SessionHdr &, // session header specified - so read it first;
    F payload,
    ExitDetectorPtr exitDetector)
{
    async_read(*ioSocketPtr, boost::asio::buffer(&sessionHdr, sizeof(sessionHdr)),
               [=] (const boost::system::error_code &errorCode,
                    size_t numOfBytes)
               {
                    if (errorCode || manager.sessionWasRemoved()) { return; }
                    initSessionSpecificSw(initSessionSpecific);
                    readRequestSw(requestCompletion, payload, exitDetector);
               });
}

template<class Cfg>
template<class F>
void Session<Cfg>::runSw(
    const NullType &,   // no session header specified - start reading
                        // the requests themselves immediately;
    F payload,
    ExitDetectorPtr exitDetector)
{
    initSessionSpecificSw(initSessionSpecific);
    readRequestSw(requestCompletion, payload, exitDetector);
}

template<class Cfg>
template<class RequestCompletion, class F>
void Session<Cfg>::readRequestSw(
    const RequestCompletion &,  // completion function instead of
                                // request header specified;
    F payload,
    ExitDetectorPtr exitDetector)
{
    readingManager.get(*ioSocketPtr, sessionHdr,
                       [=]
                       {
                            if (manager.sessionWasRemoved()) { return; }
                            processRequest(payload, exitDetector);
                       });
}

template<class Cfg>
template<class F>
void Session<Cfg>::readRequestSw(
    const NullType &,   // no reading completion function specified...
    F payload,
    ExitDetectorPtr exitDetector)
{
    // ...so read the header first...
    async_read(*ioSocketPtr, boost::asio::buffer(&requestHdr, sizeof(requestHdr)),
               [=] (const boost::system::error_code &errorCode,
                    size_t numOfBytes)
               {
                    if (errorCode || manager.sessionWasRemoved()) { return; }

                    typename Cfg::GetSizeOfRequestFromHdr getSizeOfRequestFromHdr;
                    // ...and then get a request size from the header...
                    inDataBuffer.resize(getSizeOfRequestFromHdr(requestHdr) / sizeof(typename Cfg::RequestDataRepr));

                    // ...and then read the request itself;
                    async_read(*ioSocketPtr, boost::asio::buffer(inDataBuffer),
                               [=] (const boost::system::error_code &errorCode,
                                    size_t numOfBytesRecivied)
                               {
                                    if (errorCode || manager.sessionWasRemoved()) { return; }
                                    processRequest(payload, exitDetector);
                               });
               });
}

template<class Cfg>
template<class F>
void Session<Cfg>::processRequest(
    F payload,
    ExitDetectorPtr exitDetector)
{
    outDataBuffer.clear();
    // no need to pass to a payload:
    // -- request header, if it doesn't contents anything but size of a request -
    //    user's code could see it as a size of an input buffer;
    ReplaceWithNullIf2nd<typename Cfg::RequestHdr,
                         std::is_same<typename Cfg::GetSizeOfRequestFromHdr,
                                      Network2HostLong>::value> requestHdrCorrected(requestHdr);
    // -- answer header, if it doesn't contains anything but size of an answer -
    //    by the same reason;
    ReplaceWithNullIf2nd<typename Cfg::AnswerHdr,
                         std::is_same<typename Cfg::SetSizeOfAnswer2Hdr,
                                      Host2NetworkLong>::value> answerHdrCorrected(answerHdr);
    // -- out data buffer, if proto does not provide any
    //    answer (what would user do with it at all?...);
    ReplaceWithNullIf2nd<AnswerDataBuffer, !Cfg::serverSendsAnswer> outDataBufferCorrected(outDataBuffer);

    int retCode = 0;
    try
    {
        // now throw out all the NullType-things and call
        // the user's payload code with all that remains;
        retCode = filteringAdapter(payload,
                                   sessionHdr,
                                   requestHdrCorrected.value,
                                   inDataBuffer,
                                   sessionSpecific,
                                   answerHdrCorrected.value,
                                   outDataBufferCorrected.value);
    }
    catch (...)
    {
    }

    if (manager.sessionWasRemoved()) { return; }
    if (!retCode) { manager.exitGracefull(); }
    writeAnswerSw(answerHdr, Int2Type<Cfg::serverSendsAnswer>(), payload, exitDetector);
}

template<class Cfg>
template<typename AnswerHdr, class F>
void Session<Cfg>::writeAnswerSw(
    const AnswerHdr &,
    const NoAnswerAtAll &,
    F payload,
    ExitDetectorPtr exitDetector)
{
    if (manager.endOfSessionReached()) { return; }
    readRequestSw(requestCompletion, payload, exitDetector); 
}

template<class Cfg>
template<typename AnswerHdr, class F>
void Session<Cfg>::writeAnswerSw(
    const AnswerHdr &,
    const AtLeastHeader &,
    F payload,
    ExitDetectorPtr exitDetector)
{
    typename Cfg::SetSizeOfAnswer2Hdr setSizeOfAnswer2Hdr;
    setSizeOfAnswer2Hdr(outDataBuffer.size() * sizeof(typename Cfg::AnswerDataRepr), answerHdr);
    async_write(*ioSocketPtr, boost::asio::buffer(&answerHdr, sizeof(answerHdr)),
                [=] (const boost::system::error_code &errorCode,
                     size_t)
                {
                    if(errorCode || manager.sessionWasRemoved()) { return; }
                    writeAnswerData(payload, exitDetector);
                });
}

template<class Cfg>
template<typename AnswerHdr, class F>
void Session<Cfg>::writeAnswerSw(
    const AnswerHdr &,
    const NothingIfNoData &,
    F payload,
    ExitDetectorPtr exitDetector)
{
    if (outDataBuffer.empty())
    {
        if (manager.endOfSessionReached()) { return; }
        readRequestSw(requestCompletion, payload, exitDetector);
    }
    else { writeAnswerSw(answerHdr, AtLeastHeader(), payload, exitDetector); }
}

template<class Cfg>
template<class F>
void Session<Cfg>::writeAnswerSw(
    const NullType &,
    const NothingIfNoData &,
    F payload,
    ExitDetectorPtr exitDetector)
{
    writeAnswerData(payload, exitDetector);
}

template<class Cfg>
template<class F>
void Session<Cfg>::writeAnswerData(
    F payload,
    ExitDetectorPtr exitDetector)
{
    if (!outDataBuffer.size())
    {
        if (manager.endOfSessionReached()) { return; }
        readRequestSw(requestCompletion, payload, exitDetector);
    }
    async_write(*ioSocketPtr, boost::asio::buffer(outDataBuffer),
                [=] (const boost::system::error_code &errorCode,
                     size_t numOfBytesSent)
                {
                    if (errorCode
                        || manager.sessionWasRemoved()
                        || manager.endOfSessionReached())
                    {
                        return;
                    }
                    readRequestSw(requestCompletion, payload, exitDetector);
                });
}

template<class Cfg>
template<class InitSessionSpecific>
void Session<Cfg>::initSessionSpecificSw(const InitSessionSpecific &f)
{
    filteringAdapter(f, sessionHdr, sessionSpecific);
}

}
