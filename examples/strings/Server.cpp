#include <ProtoTransformer/Server.hpp>
#include "Proto.hpp"

int usage(
    int ret,
    char *argv0)
{
    (ret ? std::cerr : std::cout) << "usage: " << argv0 << " <port> " << std::endl;
    return ret;
}

using namespace ProtoTransformer;

int doSomething(const RequestData &, AnswerData &);

int main(
    int argc,
    char **argv)
{
    if (argc < 2) { return usage(1, argv[0]); }
    int port = strtol(argv[1], 0, 0);
    if (!port)
    {
        std::cerr << "Cannot get a port number from " << argv[1] << "; " << std::endl;
        return 1;
    }

    Server<ProtoStrings>(port, doSomething);

    return 0;
}

int doSomething(
    const RequestData &inBuffer,
    AnswerData &outBuffer)
{
    outBuffer = inBuffer;
    // no need to put a '\0' to the end of an answer -
    // it is copied from a request;

    return std::string(inBuffer.begin(), inBuffer.begin() + strlen(&inBuffer[0])) != "terminate";
}
