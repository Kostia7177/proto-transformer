#pragma once

#include <string.h>
#include "Tools.hpp"
#include "AnswerCases.hpp"
#include "ThreadPoolWrapper.hpp"

#define AndAnyBase , class Base = NullType
#define DerivedFromBase : virtual public Base

template<class T AndAnyBase>
struct SessionHdrIs DerivedFromBase
{
    typedef typename EmptyOrNot<T, IsTransferable<T>::value>::type SessionHdr;
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
    typedef typename EmptyOrNot<T, IsTransferable<T>::value>::type RequestHdr;
    enum { proto = 1 };
};

template<class T AndAnyBase>
struct GetSizeOfRequestFromHdrIs DerivedFromBase
{
    typedef T GetSizeOfRequestFromHdr;
    enum { proto = 1 };
};

template<class T AndAnyBase>
struct SetSizeOfRequest2HdrIs DerivedFromBase
{
    typedef T SetSizeOfRequest2Hdr;
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
    typedef typename EmptyOrNot<T, IsTransferable<T>::value>::type AnswerHdr;
    enum { proto = 1 };
};

template<class T AndAnyBase>
struct SetSizeOfAnswer2HdrIs DerivedFromBase
{
    typedef T SetSizeOfAnswer2Hdr;
    enum { proto = 1 };
};

template<class T AndAnyBase>
struct GetSizeOfAnswerFromHdrIs DerivedFromBase
{
    typedef T GetSizeOfAnswerFromHdr;
    enum { proto = 1 };
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
    typedef ThreadPoolWrapper<SessionThreadPool> TaskManager;
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

class ReadUntilNull
{
    typedef std::vector<char> DataBuffer;

    size_t doIt(const DataBuffer &dataBuffer, size_t offset, size_t numOfBytes)
    {
        if (!numOfBytes) { return 0; }
        typedef DataBuffer::value_type DataRepr;
        DataRepr *endOfStr = (DataRepr *)memchr(&dataBuffer[offset], 0, numOfBytes - offset);
        if (endOfStr){ return (endOfStr + 1) - &dataBuffer[0]; }
        return 0;
    }
    public:
    template<typename SessionHdr>
    size_t operator()(const DataBuffer &dataBuffer, size_t offset, size_t numOfBytes, const SessionHdr &)
    {
        return doIt(dataBuffer, offset, numOfBytes);
    }
    size_t operator()(const DataBuffer &dataBuffer, size_t offset, size_t numOfBytes)
    {
        return doIt(dataBuffer, offset, numOfBytes);
    }
};
