#include "../JustSize.hpp"
#include "../../../TricksAndThings/SignatureManip/filteringAdapter.hpp"

namespace ProtoTransformer
{

template<class Cfg>
template<typename SessionHdr, class W>
void Session<Cfg>::readSessionHdrSw(
    const SessionHdr &, // session header specified - so read it first;
    W &workflow)
{
    async_read(ioSocket,
               Asio::buffer(&sessionContext.sessionHdr,
                            sizeof(sessionContext.sessionHdr)),
               workflow);
}

template<class Cfg>
template<class W>
void Session<Cfg>::readSessionHdrSw(
    const NullType &,   // no session header specified - start reading
                        // the requests themselves immediately;
    W &workflow)
{
    workflow();
}

template<class Cfg>
template<class InitSessionSpecific>
void Session<Cfg>::initSessionSpecificSw(const InitSessionSpecific &f)
{
    TricksAndThings::
    filteringAdapter(f,
                     static_cast<const typename Cfg::SessionHdr &>(sessionContext.sessionHdr),
                     sessionContext.sessionSpecific);
}

template<class Cfg>
template<class RequestCompletion, class W>
void Session<Cfg>::readRequestHdrSw(
    const RequestCompletion &,
    W &workflow)
{
    workflow();
}

template<class Cfg>
template<class W>
void Session<Cfg>::readRequestHdrSw(
    const NullType &,   // no reading completion function specified...
    W &workflow)
{
    setTimer();
    // ...so read the header first...
    async_read(ioSocket,
               Asio::buffer(&requestContext.requestHdr,
                            sizeof(requestContext.requestHdr)),
               workflow);
}

template<class Cfg>
template<class RequestCompletion, class W>
void Session<Cfg>::readRequestDataSw(
    const RequestCompletion &,  // completion function instead of
                                // request header specified;
    const W &workflow)
{
    setTimer();
    administration.readingManager.get(ioSocket,
                                      workflow,
                                      sessionContext.sessionHdr);
}

template<class Cfg>
template<class W>
void Session<Cfg>::readRequestDataSw(
    const NullType &,
    const W &workflow)
{
    // ...and then get a request size from the header...
    requestContext.inDataBuffer.resize(Cfg::RequestHdr::getSize(requestContext.requestHdr)
                                       / sizeof(typename Cfg::RequestDataRepr));

    // ...and then read the request itself;
    async_read(ioSocket,
               Asio::buffer(requestContext.inDataBuffer),
               workflow);
}

template<class Cfg>
template<class PayloadPtr>
void Session<Cfg>::processRequest(PayloadPtr payloadPtr)
{
    cancelTimer();
    requestContext.outDataBuffer.clear();

    auto buffersPrivate = Cfg::TaskManager::getPrivate(requestContext);
    administration.taskManager.schedule([=]
                                        {
                                            int retCode = 0;
                                            try
                                            {
                                                PayloadPtr keeper(payloadPtr);

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
                                                retCode = TricksAndThings::
                                                          filteringAdapter(payloadPtr->itself,
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

                                            if (!retCode) { ioSocket.shutdown(Socket::shutdown_receive); }
                                         });
}

template<class Cfg>
template<typename AnswerHdr, class W>
void Session<Cfg>::writeAnswerHdrSw(
    const AnswerHdr &,
    const NoAnswerAtAll &,
    W &workflow)
{
    workflow();
}

template<class Cfg>
template<typename AnswerHdr, class W>
void Session<Cfg>::writeAnswerHdrSw(
    const AnswerHdr &,
    const AtLeastHeader &,
    W &workflow)
{
    Cfg::AnswerHdr::setSize2(requestContext.outDataBuffer.size()
                             * sizeof(typename Cfg::AnswerDataRepr),
                             requestContext.answerHdr);

    async_write(ioSocket,
                Asio::buffer(&requestContext.answerHdr,
                             sizeof(requestContext.answerHdr)),
                workflow[W::writingHdr]);
}

template<class Cfg>
template<typename AnswerHdr, class W>
void Session<Cfg>::writeAnswerHdrSw(
    const AnswerHdr &,
    const NothingIfNoData &,
    W &workflow)
{
    if (!requestContext.outDataBuffer.empty())
    {
        writeAnswerHdrSw(requestContext.answerHdr,
                         AtLeastHeader(),
                         workflow);
    }
    else { workflow(); }
}

template<class Cfg>
template<class W>
void Session<Cfg>::writeAnswerHdrSw(
    const NullType &,
    const NothingIfNoData &,
    W &workflow)
{
    workflow();
}

template<class Cfg>
void Session<Cfg>::setTimer()
{
    readingTimeout.set([=]
                       { ioSocket.shutdown(Socket::shutdown_receive); },
                       sessionContext.sessionHdrRO,
                       sessionContext.sessionSpecific,
                       serverSpace);
}

template<class Cfg>
template<class AnswerMode, class W>
void Session<Cfg>::writeAnswerDataSw(
    const AnswerMode &,
    W &workflow)
{
    if (requestContext.outDataBuffer.size())
    {
        async_write(ioSocket,
                    Asio::buffer(requestContext.outDataBuffer),
                    workflow[W::writingData]);
    }
    else { workflow(); }
}
template<class Cfg>
template<class W>
void Session<Cfg>::writeAnswerDataSw(
    const NoAnswerAtAll &,
    W &workflow)
{
    workflow();
}

}
