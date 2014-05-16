#pragma once

#include <string.h>
#include "Wrappers.hpp"

#define AndAnyBase , class Base = NullType
#define DerivedFromBase : virtual public Base

template<class T AndAnyBase>
struct SessionHdrIs DerivedFromBase
{
    typedef T SessionHdr;
    enum { proto = 1 };
};

template<typename T AndAnyBase>
struct SessionSpecificIs DerivedFromBase
{
    typedef T SessionSpecific;
    enum { proto = 0 };
};

template<class T AndAnyBase>
struct InitSessionSpecificIs DerivedFromBase
{
    typedef T InitSessionSpecific;
    enum { proto =  0};
};

template<class T AndAnyBase>
struct RequestHdrIs DerivedFromBase
{
    typedef typename Wrappers::ForDataHeader<T>::Type RequestHdr;
    enum { proto = 1 };
};

template<typename T AndAnyBase>
struct RequestCompletionIs DerivedFromBase
{
    typedef T RequestCompletion;
    enum { proto = 1 };
};

template<typename T AndAnyBase>
struct RequestDataReprIs DerivedFromBase
{
    typedef T RequestDataRepr;
    typedef std::vector<RequestDataRepr> RequestData;
    enum { proto = 1 };
};

template<typename T AndAnyBase>
struct AnswerHdrIs DerivedFromBase
{
    typedef typename Wrappers::ForDataHeader<T>::Type AnswerHdr;
    enum { proto = 1 };
};

template<class T AndAnyBase>
struct ServerSpaceIs DerivedFromBase
{
    typedef T ServerSpace;
    enum { proto = 0 };
};

template<class T AndAnyBase>
struct AnswerCompletionIs DerivedFromBase
{
    typedef T AnswerCompletion;
    enum { proto = 1 };
};

template<typename T AndAnyBase>
struct AnswerDataReprIs DerivedFromBase
{
    typedef T AnswerDataRepr;
    typedef std::vector<AnswerDataRepr> AnswerData;
    enum { proto = 1 };
};

template<class T AndAnyBase>
struct ServerSendsAnswer DerivedFromBase
{
    enum { proto = 1 };
    static const int serverSendsAnswer = T::value;
};

template<class T AndAnyBase>
struct SessionManagerIs DerivedFromBase
{
    typedef T SessionManager;
    enum { proto = 0 };
};

template<class T AndAnyBase>
struct ServerThreadPoolIs DerivedFromBase
{
    typedef T ServerThreadPool;
    enum { proto = 0 };
};

template<class T AndAnyBase>
struct SessionThreadPoolIs DerivedFromBase
{
    typedef T SessionThreadPool;
    typedef Wrappers::ForThreadPool<SessionThreadPool> TaskManager;
    enum { proto = 0 };
};

template<class T AndAnyBase>
struct ReadingManagerIs DerivedFromBase
{
    typedef T ReadingManager;
    enum { proto = 0 };
};

template<class T AndAnyBase>
struct NumOfWorkersIs DerivedFromBase
{
    static const unsigned int numOfWorkers = T::value;
    enum { proto = 0 };
};

template<class T AndAnyBase>
struct ParallelRequestsPerSessionIs DerivedFromBase
{
    static const unsigned int numOfRequestsPerSession = T::value;
    enum { proto = 0 };
};

template<class T AndAnyBase>
struct LoggerIs DerivedFromBase
{
    typedef Wrappers::ForLogger<T> Logger;
    enum { proto = 0 };
};
