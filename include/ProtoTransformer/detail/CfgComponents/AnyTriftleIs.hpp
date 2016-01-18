#pragma once

#include "RequestHdrIs.hpp"
#include "RequestTimeoutIs.hpp"
#include <type_traits>

namespace ProtoTransformer
{

template<class T, class Base = NullType>
struct SessionHdrIs : virtual Base
{
    static_assert(IsTransferable<T>::value == true
                  || std::is_same<T, NullType>::value == true,
                  "Session header must be either something transferable (arithmetic or pod) or NullType. ");
    typedef T SessionHdr;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct SessionSpecificIs : virtual Base
{
    typedef T SessionSpecific;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct InitSessionSpecificIs : virtual Base
{
    typedef T InitSessionSpecific;
    enum { proto =  0};
};

template<typename T, class Base = NullType>
struct RequestCompletionIs : virtual Base
{
    typedef T RequestCompletion;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct RequestDataReprIs : virtual Base
{
    typedef T RequestDataRepr;
    typedef std::vector<RequestDataRepr> RequestData;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct AnswerHdrIs : virtual Base
{
    typedef typename RequestHdrIs<T, Base>  // request and answer header's wrappers are
                                            // seem to be the same, so let's reuse the
                                            // request header's wrapper;
                     ::RequestHdr AnswerHdr;
    enum { proto = 1 };
};

template<class T, class Base = NullType>
struct ServerSpaceIs : virtual Base
{
    typedef T ServerSpace;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct ClientGlobalSpaceIs : virtual Base
{
    typedef T ClientGlobalSpace;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct AnswerCompletionIs : virtual Base
{
    typedef T AnswerCompletion;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct AnswerDataReprIs : virtual Base
{
    typedef T AnswerDataRepr;
    typedef std::vector<AnswerDataRepr> AnswerData;
    enum { proto = 1 };
};

template<class T, class Base = NullType>
struct ServerSendsAnswer : virtual Base
{
    enum { proto = 1 };
    static const int serverSendsAnswer = T::value;
};

template<class T, class Base = NullType>
struct SessionManagerIs : virtual Base
{
    typedef T SessionManager;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct ServerThreadPoolIs : virtual Base
{
    typedef T ServerThreadPool;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct ReadingManagerIs : virtual Base
{
    typedef T ReadingManager;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct NumOfWorkersIs : virtual Base
{
    static const unsigned int numOfWorkers = T::value;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct ParallelRequestsPerSessionIs : virtual Base
{
    static const unsigned int numOfRequestsPerSession = T::value;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct AnswerTimeoutIs : virtual Base
{
    typedef typename RequestTimeoutIs<T, Base>  // wrappers for request and answer timeout
                                                // are seem to be the same, so let's reuse
                                                // the request timeout wrapper;
                     ::RequestTimeout AnswerTimeout;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct SigintHandlerIs : virtual Base
{
    typedef T SigintHandler;
    enum { proto = 0 };
};

}
