#include "../JustSize.hpp"
#include "../../../TricksAndThings/SignatureManip/filteringAdapter.hpp"

namespace ProtoTransformer
{

template<class Cfg>
void Session<Cfg>::throwIfRemoved()
{
    if (administration.exitManager.sessionWasRemoved())
    { throw SessionWasRemoved(); }
}

template<class Cfg>
template<typename SessionHdr>
void Session<Cfg>::readSessionHdrSw(
    SessionHdr &buf, // session header specified - so read it first;
    const YieldContext &yield)
{
    phase = readingSessionHdr;
    async_read(ioSocket,
               Asio::buffer(&buf,
                            sizeof(buf)),
               yield);
    throwIfRemoved();
}

template<class Cfg>
void Session<Cfg>::readSessionHdrSw(
    NullType &,   // no session header specified - start reading
                        // the requests themselves immediately;
    const YieldContext &)
{
}

template<class Cfg>
template<class RequestHdr>
void Session<Cfg>::readRequestHdrSw(
    RequestHdr &buf,
    const YieldContext &yield)
{
    phase = readingRequestHdr;
    setTimer();
    // ...so read the header first...
    async_read(ioSocket,
               Asio::buffer(&buf,
                            sizeof(buf)),
               yield);
    throwIfRemoved();
}

template<class Cfg>
void Session<Cfg>::readRequestHdrSw(
    NullType &,
    const YieldContext &)
{
}

template<class Cfg>
template<class RequestCompletion>
void Session<Cfg>::readRequestDataSw(
    const RequestCompletion &,  // completion function instead of
                                // request header specified;
    const YieldContext &yield)
{
    setTimer();
    phase = readingRequestData;
    administration.readingManager.get(ioSocket,
                                      yield,
                                      sessionContext.sessionHdr);
    throwIfRemoved();
}

template<class Cfg>
void Session<Cfg>::readRequestDataSw(
    const NullType &,   // no reading completion function specified...
    const YieldContext &yield)
{
    requestContext.inDataBuffer.resize(Cfg::RequestHdr::getSize(requestContext.requestHdr)
                                       / sizeof(typename Cfg::RequestDataRepr));
    phase = readingRequestData;
    async_read(ioSocket,
               Asio::buffer(requestContext.inDataBuffer),
               yield);
    throwIfRemoved();
}

template<class Cfg>
template<class F>
void Session<Cfg>::processRequest(PayloadPtr<F> payloadPtr)
{
    cancelTimer();
    requestContext.outDataBuffer.clear();

    auto buffersPrivate = Cfg::TaskManager::getPrivate(requestContext);
    administration.taskManager.schedule([=]
                                        {
                                            int retCode = 0;
                                            try
                                            {
                                                PayloadPtr<F> keeper(payloadPtr);

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
    throwIfRemoved();
}

template<class Cfg>
template<typename AnswerHdr>
void Session<Cfg>::writeAnswerHdrSw(
    AnswerHdr &,
    const NoAnswerAtAll &,
    const YieldContext &)
{
}

template<class Cfg>
template<typename AnswerHdr>
void Session<Cfg>::writeAnswerHdrSw(
    AnswerHdr &hdr,
    const AtLeastHeader &,
    const YieldContext &yield)
{
    Cfg::AnswerHdr::setSize2(requestContext.outDataBuffer.size()
                             * sizeof(typename Cfg::AnswerDataRepr),
                             hdr);

    phase = writingAnswerHdr;
    async_write(ioSocket,
                Asio::buffer(&hdr,sizeof(hdr)),
                yield);
    throwIfRemoved();
}

template<class Cfg>
template<typename AnswerHdr>
void Session<Cfg>::writeAnswerHdrSw(
    AnswerHdr &hdr,
    const NothingIfNoData &,
    const YieldContext &yield)
{
    if (!requestContext.outDataBuffer.empty())
    {
        writeAnswerHdrSw(hdr,
                         AtLeastHeader(),
                         yield);
    }
}

template<class Cfg>
void Session<Cfg>::writeAnswerHdrSw(
    NullType &,
    const NothingIfNoData &,
    const YieldContext &)
{
}

template<class Cfg>
template<class Condition>
void Session<Cfg>::writeAnswerData(
    const Condition &,
    const YieldContext &yield)
{
    if (requestContext.outDataBuffer.size())
    {
        phase = writingAnswerData;
        async_write(ioSocket,
                    Asio::buffer(requestContext.outDataBuffer),
                    yield);
        throwIfRemoved();
    }
}

template<class Cfg>
void Session<Cfg>::writeAnswerData(
    const NoAnswerAtAll &,
    const YieldContext &)
{
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
void Session<Cfg>::setTimer()
{
    readingTimeout.set([=]
                       { ioSocket.shutdown(Socket::shutdown_receive); },
                       sessionContext.sessionHdrRO,
                       sessionContext.sessionSpecific,
                       serverSpace);
}

template<class Cfg>
template<class F>
void Session<Cfg>::run(F payload)
{
    PayloadPtr<F> payloadPtr(new Payload<F>(payload,
                                            this->shared_from_this()));
    Asio::spawn(ioSocket.get_io_service(),
                [=](YieldContext yield)
                {
                    try
                    {
                        readSessionHdrSw(sessionContext.sessionHdr, yield);
                        initSessionSpecificSw(sessionContext.initSessionSpecific);
                        while (1)
                        {
                            readRequestHdrSw(requestContext.requestHdr, yield);
                            readRequestDataSw(requestCompletion, yield);
                            processRequest(payloadPtr);

                            writeAnswerHdrSw(requestContext.answerHdr,
                                             Int2Type<Cfg::serverSendsAnswer>(),
                                             yield);
                            writeAnswerData(Int2Type<Cfg::serverSendsAnswer>(), yield);
                        }
                    }
                    catch (SessionWasRemoved &) {}
                    catch (std::exception &exc)
                    {
                        logger(logger.errorOccured(), "%s: '%s';",
                               errorMessages[phase].c_str(), exc.what());
                    }
                });
}

}
