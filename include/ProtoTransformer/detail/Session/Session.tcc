#include "../ParamPackManip/filteringAdapter.hpp"
#include "../JustSize.hpp"

namespace ProtoTransformer
{

template<class Cfg>
template<class F>
void Session<Cfg>::run(F payload)
{
    typedef typename Cfg::SessionManager::template ExitDetector<Session> ExitDetector;
    runSw(sessionContext.sessionHdr, payload, std::shared_ptr<ExitDetector>(new ExitDetector(this->shared_from_this())));
}

template<class Cfg>
template<typename SessionHdr, class F>
void Session<Cfg>::runSw(
    const SessionHdr &, // session header specified - so read it first;
    F payload,
    ExitDetectorPtr exitDetector)
{
    async_read(*ioSocketPtr,
               Asio::buffer(&sessionContext.sessionHdr, sizeof(sessionContext.sessionHdr)),
               [=] (const Sys::error_code &errorCode,
                    size_t numOfBytes)
               {
                    if (administration.exitManager.sessionWasRemoved()) { return; }
                    if (errorCode)
                    {
                        logger(logger.errorOccured(),
                               "Cannot read session header: '%s'; ", errorCode.message().c_str());
                        return;
                    }
                    initSessionSpecificSw(sessionContext.initSessionSpecific);
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
    initSessionSpecificSw(sessionContext.initSessionSpecific);
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
    setTimer();
    administration.readingManager.get(*ioSocketPtr,
                                      [=]
                                      {
                                        if (administration.exitManager.sessionWasRemoved()) { return; }
                                        processRequest(payload, exitDetector);
                                      } ,
                                      sessionContext.sessionHdr);
}

template<class Cfg>
template<class F>
void Session<Cfg>::readRequestSw(
    const NullType &,   // no reading completion function specified...
    F payload,
    ExitDetectorPtr exitDetector)
{
    setTimer();
    // ...so read the header first...
    async_read(*ioSocketPtr,
               Asio::buffer(&requestContext.requestHdr, sizeof(requestContext.requestHdr)),
               [=] (const Sys::error_code &errorCode,
                    size_t numOfBytes)
               {
                    if (administration.exitManager.sessionWasRemoved()) { return; }
                    if (errorCode)
                    {
                        logger(logger.errorOccured(), "Cannot read request header '%s'; ", errorCode.message().c_str());
                        return;
                    }

                    // ...and then get a request size from the header...
                    requestContext.inDataBuffer.resize(Cfg::RequestHdr::getSize(requestContext.requestHdr) / sizeof(typename Cfg::RequestDataRepr));

                    // ...and then read the request itself;
                    async_read(*ioSocketPtr ,
                               Asio::buffer(requestContext.inDataBuffer),
                               [=] (const Sys::error_code &errorCode,
                                    size_t numOfBytesRecivied)
                               {
                                    if (administration.exitManager.sessionWasRemoved()) { return; }
                                    if (errorCode)
                                    {
                                        logger(logger.errorOccured(), "Cannot read request data '%s'; ", errorCode.message().c_str());
                                        return;
                                    }
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
    cancelTimer();
    requestContext.outDataBuffer.clear();

    auto buffersPrivate = Cfg::TaskManager::getPrivate(requestContext);
    administration.taskManager.schedule([=]
                                        {
                                            int retCode = 0;
                                            try
                                            {
                                                ExitDetectorPtr keeper(exitDetector);

                                                // no need to pass to a payload:
                                                // -- request header, if it doesn't contents anything but size of a request -
                                                //    user's code could see it as a size of an input buffer;
                                                typedef ReplaceWithNullIf2nd<typename Cfg::RequestHdr::Itself,
                                                                             std::is_same<typename Cfg::RequestHdr, JustSize>::value
                                                                            > RequestHdrCorrected;
                                                RequestHdrCorrected requestHdrCorrected(buffersPrivate->requestHdr);
                                                // -- answer header, if it doesn't contains anything but size of an answer -
                                                //    by the same reason;
                                                ReplaceWithNullIf2nd<typename Cfg::AnswerHdr::Itself,
                                                                     std::is_same<typename Cfg::AnswerHdr, JustSize
                                                                                 >::value
                                                                    > answerHdrCorrected(buffersPrivate->answerHdr);
                                                // -- out data buffer, if proto does not provide any
                                                //    answer (what would user do with it at all?...);
                                                ReplaceWithNullIf2nd<typename RequestContext<Cfg>::OutData,
                                                                     Cfg::serverSendsAnswer == never
                                                                    > outDataBufferCorrected(buffersPrivate->outDataBuffer);

                                                // now throw out all the NullType-things and call
                                                // the user's payload code with all that remains;
                                                retCode = filteringAdapter(payload,
                                                                           sessionContext.sessionHdrRO,
                                                                           // turn the invariants to const;
                                                                           static_cast<const typename RequestHdrCorrected::
                                                                                                      Type &>(requestHdrCorrected.value),
                                                                           static_cast<const typename RequestContext<Cfg>::
                                                                                                      InData &>(buffersPrivate->inDataBuffer),
                                                                           sessionContext.sessionSpecific,
                                                                           answerHdrCorrected.value,
                                                                           outDataBufferCorrected.value,
                                                                           serverSpace);

                                            }
                                            catch(const std::exception &exc)
                                            {
                                                logger(logger.payloadCrached(),
                                                       "Payload function have thrown an exception %s; ",
                                                       exc.what());
                                            }
                                            catch (...)
                                            {
                                                logger(logger.payloadCrached(),
                                                       "Payload function have thrown an unrecognized exception; ");
                                            }

                                            if (!retCode) { ioSocketPtr->shutdown(Socket::shutdown_receive); }
                                         });

    if (administration.exitManager.sessionWasRemoved()) { return; }
    writeAnswerSw(requestContext.answerHdr, Int2Type<Cfg::serverSendsAnswer>(), payload, exitDetector);
}

template<class Cfg>
template<typename AnswerHdr, class F>
void Session<Cfg>::writeAnswerSw(
    const AnswerHdr &,
    const NoAnswerAtAll &,
    F payload,
    ExitDetectorPtr exitDetector)
{
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
    Cfg::AnswerHdr::setSize2(requestContext.outDataBuffer.size() * sizeof(typename Cfg::AnswerDataRepr), requestContext.answerHdr);

    async_write(*ioSocketPtr, Asio::buffer(&requestContext.answerHdr, sizeof(requestContext.answerHdr)),
                [=] (const Sys::error_code &errorCode,
                     size_t)
                {
                    if (administration.exitManager.sessionWasRemoved()) { return; }
                    if (errorCode)
                    {
                        logger(logger.errorOccured(), "Cannot write answer header '%s'; ", errorCode.message().c_str());
                        return;
                    }
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
    if (requestContext.outDataBuffer.empty())
    {
        readRequestSw(requestCompletion, payload, exitDetector);
    }
    else { writeAnswerSw(requestContext.answerHdr, AtLeastHeader(), payload, exitDetector); }
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
    if (!requestContext.outDataBuffer.size())
    {
        readRequestSw(requestCompletion, payload, exitDetector);
    }
    else
    {
        async_write(*ioSocketPtr, Asio::buffer(requestContext.outDataBuffer),
                    [=] (const Sys::error_code &errorCode,
                         size_t numOfBytesSent)
                    {
                        if (administration.exitManager.sessionWasRemoved()) { return; }
                        if (errorCode)
                        {
                            logger(logger.errorOccured(), "Cannot write answer data '%s'; ", errorCode.message().c_str());
                            return;
                        }
                        readRequestSw(requestCompletion, payload, exitDetector);
                });
    }
}

template<class Cfg>
template<class InitSessionSpecific>
void Session<Cfg>::initSessionSpecificSw(const InitSessionSpecific &f)
{
    filteringAdapter(f, static_cast<const typename Cfg::SessionHdr &>(sessionContext.sessionHdr), sessionContext.sessionSpecific);
}
template<class Cfg>
void Session<Cfg>::setTimer()
{
    readingTimeout.set([=]
                       {
                            ioSocketPtr->shutdown(Socket::shutdown_receive);
                       },
                       sessionContext.sessionHdrRO,
                       sessionContext.sessionSpecific,
                       serverSpace);
}

}
