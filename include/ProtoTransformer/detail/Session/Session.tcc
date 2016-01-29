#include "../JustSize.hpp"
#include "../../../TricksAndThings/SignatureManip/filteringAdapter.hpp"

namespace ProtoTransformer
{

#include<boost/asio/yield.hpp>
template<class Cfg>
template<class F>
void Session<Cfg>::workflow(Tappet<F> tappet)
{
    if (goOn)
    {
        reenter(this)
        {
            yield readSessionHdrSw(sessionContext.sessionHdr,
                                   tappet.atPhase(readingSessionHdr));

            initSessionSpecificSw(sessionContext.initSessionSpecific);

            do
            {
                yield readRequestHdrSw(requestCompletion,
                                       tappet.atPhase(readingHdr));

                yield readRequestDataSw(requestCompletion,
                                        tappet.atPhase(readingData));

                processRequest(tappet.payloadPtr);

                typedef Int2Type<Cfg::serverSendsAnswer> AnswerMode;
                yield writeAnswerHdrSw(requestContext.answerHdr, AnswerMode(),
                                       tappet.atPhase(writingHdr));

                yield writeAnswerDataSw(AnswerMode(),
                                        tappet.atPhase(writingData));
            }
            while (goOn);
        }
    }
}
#include<boost/asio/unyield.hpp>

template<class Cfg>
template<typename SessionHdr, class F>
void Session<Cfg>::readSessionHdrSw(
    const SessionHdr &, // session header specified - so read it first;
    Tappet<F> tappet)
{
    async_read(*ioSocketPtr,
               Asio::buffer(&sessionContext.sessionHdr,
                            sizeof(sessionContext.sessionHdr)),
               tappet);
}

template<class Cfg>
template<class F>
void Session<Cfg>::readSessionHdrSw(
    const NullType &,   // no session header specified - start reading
                        // the requests themselves immediately;
    Tappet<F> tappet)
{
    tappet.kickWorkflow();
}
template<class Cfg>
template<class RequestCompletion, class F>
void Session<Cfg>::readRequestHdrSw(
    const RequestCompletion &,
    Tappet<F> tappet)
{
    tappet.kickWorkflow();
}

template<class Cfg>
template<class F>
void Session<Cfg>::readRequestHdrSw(
    const NullType &,   // no reading completion function specified...
    Tappet<F> tappet)
{
    setTimer();
    // ...so read the header first...
    async_read(*ioSocketPtr,
               Asio::buffer(&requestContext.requestHdr,
                            sizeof(requestContext.requestHdr)),
               tappet);
}

template<class Cfg>
template<class RequestCompletion, class F>
void Session<Cfg>::readRequestDataSw(
    const RequestCompletion &,  // completion function instead of
                                // request header specified;
    Tappet<F> tappet)
{
    setTimer();
    administration.readingManager.get(*ioSocketPtr,
                                      tappet,
                                      sessionContext.sessionHdr);
}

template<class Cfg>
template<class F>
void Session<Cfg>::readRequestDataSw(
    const NullType &,
    Tappet<F> tappet)
{
    // ...and then get a request size from the header...
    requestContext.inDataBuffer.resize(Cfg::RequestHdr::getSize(requestContext.requestHdr)
                                       / sizeof(typename Cfg::RequestDataRepr));

    // ...and then read the request itself;
    async_read(*ioSocketPtr,
               Asio::buffer(requestContext.inDataBuffer),
               tappet);
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

                                            if (!retCode) { ioSocketPtr->shutdown(Socket::shutdown_receive); }
                                         });
}

template<class Cfg>
template<typename AnswerHdr, class F>
void Session<Cfg>::writeAnswerHdrSw(
    const AnswerHdr &,
    const NoAnswerAtAll &,
    Tappet<F> tappet)
{
    tappet.kickWorkflow();
}

template<class Cfg>
template<typename AnswerHdr, class F>
void Session<Cfg>::writeAnswerHdrSw(
    const AnswerHdr &,
    const AtLeastHeader &,
    Tappet<F> tappet)
{
    Cfg::AnswerHdr::setSize2(requestContext.outDataBuffer.size()
                             * sizeof(typename Cfg::AnswerDataRepr),
                             requestContext.answerHdr);

    async_write(*ioSocketPtr,
                Asio::buffer(&requestContext.answerHdr,
                             sizeof(requestContext.answerHdr)),
                tappet);
}

template<class Cfg>
template<typename AnswerHdr, class F>
void Session<Cfg>::writeAnswerHdrSw(
    const AnswerHdr &,
    const NothingIfNoData &,
    Tappet<F> tappet)
{
    if (!requestContext.outDataBuffer.empty())
    {
        writeAnswerHdrSw(requestContext.answerHdr,
                         AtLeastHeader(),
                         tappet);
    }
    else { tappet.kickWorkflow(); }
}

template<class Cfg>
template<class F>
void Session<Cfg>::writeAnswerHdrSw(
    const NullType &,
    const NothingIfNoData &,
    Tappet<F> tappet)
{
    tappet.kickWorkflow();
}

template<class Cfg>
template<class AnswerMode, class F>
void Session<Cfg>::writeAnswerDataSw(
    const AnswerMode &,
    Tappet<F> tappet)
{
    if (requestContext.outDataBuffer.size())
    {
        async_write(*ioSocketPtr,
                    Asio::buffer(requestContext.outDataBuffer),
                    tappet);
    }
    else { tappet.kickWorkflow(); }
}
template<class Cfg>
template<class F>
void Session<Cfg>::writeAnswerDataSw(
    const NoAnswerAtAll &,
    Tappet<F> tappet)
{
    tappet.kickWorkflow();
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
                       {
                            ioSocketPtr->shutdown(Socket::shutdown_receive);
                       },
                       sessionContext.sessionHdrRO,
                       sessionContext.sessionSpecific,
                       serverSpace);
}

}
