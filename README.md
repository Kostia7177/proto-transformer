proto-transformer
=================

###What is this

Easy way to make and customize your own application-layer protocol. Just play with it like with a robot-transformer.

###Requires

Boost threadpool: http://threadpool.sourceforge.net/ .

###Installation

Simply copy to /usr/local/include, for example.

cp -R include/ProtoTransformer/ /usr/local/include/

###How to use.

**First**, create a header and define your protocol within it. Suppose, we want to replace the default value of policy "request data representation" (which is unsigned char) with char, and for "answer data representation" set it to int.

```cplusplus
// YourOwnProto.h file

#include <ProtoTransformer/Proto.hpp>

typedef ProtoTransformer::Proto
    <
        UsePolicy<RequestDataReprIs, char>,
        UsePolicy<AnswerDataReprIs, int>
    > YourOwnProto;
```

This new-defined type (actually is a union of traits) must be the first template parameter both of server and client (see below) .
Policies in the template parameter list are order-independent.
Full list of available protocol policies are available at the appendix A.

**Second**, create your server. Suppose, you've wrote the function 'payload' that takes vector<char> as a request and provides an answer as vector<int> . So,

```cplusplus
// YourServer.cpp file;

#include <ProtoTransformer/Server.hpp>
#include "YourOwnProto.hpp"

int payload(const vector<char> &request, vector<int> &answer);

int main(int argc, char **argv)
{
        typedef ProtoTransformer::
                Server<
                        YourOwnProto,
                        UsePolicy<SessionManagerIs, SessionManagerWithMap>
                      > YourOwnServer;

        YourOwnServer(4242, payload);   // server will listen a 4242 port
                                        // and call a payload on each request

        return 0;
}
```
Note that the first template parameter in server definition must be a protocol.
Other parameters (non-protocol tunings, example above shows just one - "session manager" ) are order-independent. Full list of available non-protocol
tunings see at the appendix B. Read 'Server payload signature rules' to keep accordance between it and a protocol.

And **third**. Create a client.
```cplusplus
// YourOwnClient.cpp file

#include <ProtoTransformer/Server.hpp>
#include "YourOwnProto.hpp"

int main(int argc, char **argv)
{
        ProtoTransformer::Client<YourOwnProto> client("host.with.your.server.com", 4242);

        while(1)
        {
                vector<char> requestBuffer();
                // fill a request buffer
                vector<int> answer = client.request(requestBuffer);
                // do something with an answer;
        }

        return 0;
}
```
Much more expressive (but more verbose) examples you can find at the 'examples' directory.

###Server payload signature rules
What the data could be passed by the server's code to it's payload?
First, of corse, the request body itself. So,

+ const vector<RequestDataRepr> &requestData

There can be any header preceeding the request. If the header contains only a size of the request, no need to pass it to a user's code - size of the request is available as request.size() . If the request header is exactly of 'JustSize' type (see JustSize.hpp file) we assert that nothing but size there inside the request header. Otherwise we suppose that the request header contains something more than simply size. What could be there - time of request forming, sequence number of it, it's id or something else. So, in this case we have to pass it to the payload:

+ const RequestHdr &requestHdr **<--**
+ const vector<RequestDataRepr> requestData

Request header is actually a wrapper around header's type itself (typedefed as **Itself** ) and two static functions, which are to get and set value of size into the request itself (excuse me this extra itselfing) . Functions must have names **getSize** and **setSize2** the following interfaces
```cplusplus
static uint32_t getSize(Itself)
static void setSize2(uint32_t, Itself &);
```

Now, if a session header is not 'NullType', we must pass it too.

+ const SessionHdr &sessionHeader **<--**
+ const RequestHdr &requestHdr
+ const vector<RequestDataRepr> requestData

There is an ability to keep any cross-requests data, that lives through all the session lifetime, private for this session and available for all it's requests. So -

+ const SessionHdr &sessionHeader
+ const RequestHdr &requestHdr
+ const vector<RequestDataRepr> request
+ SessionSpecific &sessionSpecific **<--**

Now. Usually, session returns any answer to a client. So, if no NoAnswerAtAll policy specified, payload code will receive the reference to an empty vector wich is to be filled by asnwer data:

+ const SessionHdr &sessionHeader
+ const RequestHdr &requestHdr
+ const vector<RequestDataRepr> requestData
+ SessionSpecific &sessionSpecific
+ vector<AnswerDataRepr> &answerData **<--** where to put an answer

Answer data (the same as request data) can be preceeded by header.

+ const SessionHdr &sessionHeader
+ const RequestHdr &requestHdr
+ const vector<RequestDataRepr> requestData
+ SessionSpecific &sessionSpecific
+ AnswerHdr &answerHdr **<--** where to put answer header
+ vector<AnswerDataRepr> &answerData

If any objects of a server address space are desired to be available for all requests of all sessions, it is recommended to aggregate them to any structure and to pass to a payload code as one object (see suchATricky1 for example) .

1. const SessionHdr &sessionHeader
1. const RequestHdr &requestHdr
1. const vector<RequestDataRepr> requestData
1. SessionSpecific &sessionSpecific
1. AnswerHdr &answerHdr
1. vector<AnswerDataRepr> &answerData
1. address of a "server space buffer" **<--**

Remember the preceedence order of theese items.

So, if payload code takes only a request (and no any headers) , not uses session-specific data and not provides any answer, it's sgnature will
```cplusplus
int payload(const vector<RequestDataRepr> &);
```
and if all theese abilities are used, user's code must have the signature

```cplusplus
int payload(const SessionHdr &,
            const RequestHdr &,
            const vector<RequestReprData> &,
            SessionSpecific &,
            AnswerHdr &,
            vector<AnswerDataRepr> &,
            ServerSpace *);
```
With the exception of requestData, all other arguments can be omitted - depend on protocol components. But the relative order (preceedence) of them is still the same.

###Appendix A. Protocol components.

Angle bracets are containing a default pre-set value.

####Whole session description
- SessionHdrIs\<NullType>	- any session invariant; passed to a server after connection will be established at the very beginning of the session;

####Request description
- RequestHdrIs\<JustSize>		- traits-structure with typedef of a request header itself, and two static functions that are get and set size from/to header itself, see JustSize.hpp file for example; by default header itself is uint32_t and is to be calculated automatically as a size of request data buffer get/set functions are wrapped ntohl and htonl;
- RequestCompletionIs\<NullType>	- user can specify a request-completion function instead of request header;
					  note that the request header must be turned to NullType in this case explicitly;
- RequestDataReprIs\<unsigned char>	- value type of a request vector;

####Answer description
- ServerSendsAnswer\<AtLeastHeader> - does server replies anything, and how much, if so;

1. AtLeastHeader	- by default - if the answer contains no data, just a header will be returned to a client (signalling that no data will follow);
1. NoAnswerAtAll	- alternately, there is case when no answer supposed at all - requests-only protocol;
1. NothingIfNoData	- and a case sends nothing if no answer data - even header;

- Answer2HdrIs\<JustSize>
- AnswerCompletionIs\<NullType>
- AnswerDataReprIs\<unsigned char>

###Appendix B. Non-protocol components.

- NumOfWorkersIs\<Int2Type\<hardwareConcurrency>>	- number of parallel sessions;
- ParallelRequestsPerSessionIs\<Int2Type\<1>>	- has a meaning if SessionThreadPoolIs
						  points to something multi-threaded
						  (boost::threadpool, for example)
						  note, that 0 means thread concurrency!
- SessionSpecificIs\<NullType>			- such a session-static variable(s); some
						  data that is available for all requests
						  within a session (and is not available
						  for all the other sessions requests);
- InitSessionSpecificIs\<NullType>		- function that initializes session
						  specific (may be by session-header
						  data or it's part);
- SessionManagerIs\<EmptyManager>		- empty (by default); starts the session
						  and forget about it - session will be
						  dropped when it'll be finished independently
						  of a server; see SessionManagers/Empty.hpp;
						  alternatively there could be used a manager
						  with map that remembers all the sessions and
						  terminates them on server's exit; see
						  SessionManagers/WithMap.hpp;
- ServerThreadPoolIs\<boost::threadpool::pool>
- SessionThreadPoolIs\<NullType>
- ReadingManagerIs\<ReadingManager>		- that thing, that manages the reading
						  when no request size is known (not a
						  completion function! just what it calls!
						  see ReadingManager.hpp file);
- LoggerIs\<NullType> - where the error/warning/debug messages will be written;
1. NullType - nowhere all theese messages will be written;
2. SyslogLogger - messages will be directed to syslog (see Loggers/Syslog.hpp file);
3. StderrLogger - messages will be directed to stderr (see Loggers/Stderr.hpp file)
