proto-transformer
=================

#How to use.

**First**, create a header with your own protocol by typedefing the ProtoTransformer::Proto template:

```cpluspllus
// YourOwnProto.h file

#include <ProtoTransformer/Proto.hpp>

typedef ProtoTransformer::Proto
    <
	// just for example:
	UsePolicy<RequestDataReprIs, char>	// request will be a vector<char> now
	// ... etc;
    > YourOwnProto;

// end of YourOwnProto.h file;
```

Order of policies is random.
Full list of available protocol policies available at the appendix A.

**Second**, create your server:

```cplusplus
// YourServer.cpp file;

#include <ProtoTransformer/Server.hpp>
#include "YourOwnProto.hpp"

int payload(const vector<char> &request, vector<unsigned char> &answer);

int main(int argc, char **argv)
{
	typedef ProtoTransformer::
		Server<
			// first template parameter must be a protocol;
	                YourOwnProto,
			// for example again, after a protocol there may be some non-protocol
			// tunings in random order;
	                UsePolicy<SessionManagerIs, SessionManagerWithMap>
			// ...etc;
                      > YourOwnServer;

	YourOwnServer(4242, payload);	// server will listen a 4242 port
					// and call a payload on each request

	return 0;
}

// end of YourServer.cpp file;
```
Note that the first template parameter in server definition must be a protocol.
Other (non-protocol parameters) may be follow in a random order. Full list of available non-protocol
tunings sees at the appendix B.
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
		vector<unsigned char> answer = client.request(requestBuffer);
		// do something with an answer;
	}

	return 0;
}

// end of YourOwnClient.cpp file
```
#Appendix A. Protocol components.

Angle bracets contain a default pre-set value.

	Whole session description
	- SessionHdrIs<NullType>	- any session invariant; passed to a server
					  after connection will be established at the
					  very beginning of the session;

	Request description
	- RequestHdrIs<PureHdr>		- by default is uint32_t and is to be calculated,
					  automatically as a size of request data buffer;
	- GetSizeOfRequestFromHdrIs<Network2HostLong>	- by default is ntohl();
	- SetSizeOfRequest2HdrIs<Host2NetworkLong>	- by default is htonl();
	- RequestCompletionIs<NullType>	- user can specify a request-completion
					  function instead of request header;
					  note that the request header must be
					  turned to NullType in this case
					  explicitly;
	- RequestDataReprIs<unsigned char>	- value type of a request vector;

	Answer description
	- ServerSendsAnswer<AtLeastHeader>	- by default - if the answer contains no
						  data, just a header will be returned
						  to a client (signalling that no data
						  will follow);
			    NoAnswerAtAll	- alternately, there is case when no
						  answer supposed at all - requests-only
						  protocol;
			    NothingIfNoData	- and a case sends nothing if
						  no answer data - even header;
	- SetSizeOfAnswer2HdrIs<Host2NetworkLong>
	- GetSizeOfAnswerFromHdrIs<Network2HostLong>
	- AnswerCompletionIs<NullType>
	- AnswerDataReprIs<unsigned char>

#Appendix B. Non-protocol components.

	- NumOfWorkersIs<Int2Type<hardwareConcurrency>>	- number of parallel sessions;
	- ParallelRequestsPerSessionIs<Int2Type<1>>	- has a meaning if SessionThreadPoolIs
							  points to something multi-threaded
							  (boost::threadpool, for example)
							  note, that 0 means thread concurrency!
	- SessionSpecificIs<NullType>			- such a session-static variable(s); some
							  data that is available for all requests
							  within a session (and is not available
							  for all the other sessions requests);
	- InitSessionSpecificIs<NullType>		- function that initializes session
							  specific (may be by session-header
							  data or it's part);
	- SessionManagerIs<EmptyManager>		- empty (by default); starts the session
							  and forget about it - session will be
							  dropped when it'll be finished independently
							  of a server; see SessionManagers/Empty.hpp;
							  alternatively there could be used a manager
							  with map that remembers all the sessions and
							  terminates them on server's exit; see
							  SessionManagers/WithMap.hpp;
	- ServerThreadPoolIs<boost::threadpool::pool>
	- SessionThreadPoolIs<NullType>
	- ReadingManagerIs<ReadingManager>		- that thing, that manages the reading
							  when no request size is known (not a
							  completion function! just what it calls!
							  see ReadingManager.hpp file);
