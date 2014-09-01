#pragma once

#include <vector>
#include "../AnswerCases.hpp"

template<class Cfg>
struct RequestContext
{   // a component of a 'Session' class;
    // data that is changing with every request
    // and is to be private of a request processing;
    // (privacy is actual with parallel request
    // processing, i.e. with parallelRequestsPerSession != 1
    // and SessionThreadPool != NullType);
    //
    typename Cfg::RequestHdr::Itself requestHdr;    // -- request description;
                                                    //    in main case contains
                                                    //    a simple size of a request;
                                                    //    if it so, not will be
                                                    //    passed to a payload code;
    typedef std::vector<typename Cfg::RequestDataRepr> InData;
    InData inDataBuffer;                            // -- request data itself;
    typename Cfg::AnswerHdr::Itself answerHdr;      // -- the same as requestHdr, but
                                                    //    for answer;
    typedef std::vector<typename Cfg::AnswerDataRepr> OutData;
    OutData outDataBuffer;                           // -- guess what :)

    template<int any>
    void Swap(RequestContext &data, const Int2Type<any> &)
    {   // providing a private copy of a request and answer
        // data to pass it to a payload (used within parallel
        // request processing);
        requestHdr = data.requestHdr;               // in most cases is a something
                                                    // simple, so - simple copy it;
        std::swap(inDataBuffer, data.inDataBuffer); // is a vector - it's better to
                                                    // swap it;
        answerHdr = data.answerHdr;                 // and the same for answer header;
        std::swap(outDataBuffer, data.outDataBuffer);   // ...and data;
    }

    enum { serverSendsAnswer = Cfg::serverSendsAnswer };
    void Swap(RequestContext &data, const Int2Type<never> &)
    {   // omit answer buffers copying/swapping
        // if they are not used;
        requestHdr = data.requestHdr;
        std::swap(inDataBuffer, data.inDataBuffer);
    }
};
