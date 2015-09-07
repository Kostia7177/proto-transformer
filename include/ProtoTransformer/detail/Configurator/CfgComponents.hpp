#pragma once

#include <string.h>
#include "../Wrappers/ForDataHeader.hpp"
#include "../Wrappers/ForThreadPool.hpp"
#include "../Wrappers/ForLogger.hpp"
#include "../Wrappers/ForTimer.hpp"

namespace ProtoTransformer
{

template<class T, class Base = NullType>
struct SessionHdrIs : virtual public Base
{
    static_assert(IsTransferable<T>::value == true
                  || std::is_same<T, NullType>::value == true,
                  "Session header must be either something transferable (arithmetic or pod) or NullType. ");
    typedef T SessionHdr;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct SessionSpecificIs : virtual public Base
{
    typedef T SessionSpecific;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct InitSessionSpecificIs : virtual public Base
{
    typedef T InitSessionSpecific;
    enum { proto =  0};
};

template<class T, class Base = NullType>
struct RequestHdrIs : virtual public Base
{
    typedef typename Wrappers::ForDataHeader<T>::Type RequestHdr;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct RequestCompletionIs : virtual public Base
{
    typedef T RequestCompletion;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct RequestDataReprIs : virtual public Base
{
    typedef T RequestDataRepr;
    typedef std::vector<RequestDataRepr> RequestData;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct AnswerHdrIs : virtual public Base
{
    typedef typename Wrappers::ForDataHeader<T>::Type AnswerHdr;
    enum { proto = 1 };
};

template<class T, class Base = NullType>
struct ServerSpaceIs : virtual public Base
{
    typedef T ServerSpace;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct ClientGlobalSpaceIs : virtual public Base
{
    typedef T ClientGlobalSpace;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct AnswerCompletionIs : virtual public Base
{
    typedef T AnswerCompletion;
    enum { proto = 1 };
};

template<typename T, class Base = NullType>
struct AnswerDataReprIs : virtual public Base
{
    typedef T AnswerDataRepr;
    typedef std::vector<AnswerDataRepr> AnswerData;
    enum { proto = 1 };
};

template<class T, class Base = NullType>
struct ServerSendsAnswer : virtual public Base
{
    enum { proto = 1 };
    static const int serverSendsAnswer = T::value;
};

template<class T, class Base = NullType>
struct SessionManagerIs : virtual public Base
{
    typedef T SessionManager;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct ServerThreadPoolIs : virtual public Base
{
    typedef T ServerThreadPool;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct SessionThreadPoolIs : virtual public Base
{
    typedef T SessionThreadPool;
    typedef Wrappers::ForThreadPool<SessionThreadPool> TaskManager;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct ReadingManagerIs : virtual public Base
{
    typedef T ReadingManager;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct NumOfWorkersIs : virtual public Base
{
    static const unsigned int numOfWorkers = T::value;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct ParallelRequestsPerSessionIs : virtual public Base
{
    static const unsigned int numOfRequestsPerSession = T::value;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct LoggerIs : virtual public Base
{
    typedef Wrappers::ForLogger<T> Logger;
    enum { proto = 0 };
};

template<class T , class Base = NullType>
struct RequestTimeoutIs : virtual public Base
{
    typedef typename Wrappers::ForTimer<T> RequestTimeout;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct AnswerTimeoutIs : virtual public Base
{
    typedef typename Wrappers::ForTimer<T> AnswerTimeout;
    enum { proto = 0 };
};

template<class T, class Base = NullType>
struct SigintHandlerIs : virtual public Base
{
    typedef T SigintHandler;
    enum { proto = 0 };
};

}
