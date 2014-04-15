#include <ProtoTransformer/Server.hpp>
#include <boost/algorithm/string.hpp>
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

    Server<ProtoSimplest>(port, doSomething);

    return 0;
}

int doSomething(const RequestData &inBuffer, AnswerData &outBuffer)
{
    std::string inBufStr = std::string(inBuffer.begin(), inBuffer.end());
    boost::to_upper(inBufStr);

    outBuffer = AnswerData(inBufStr.begin(), inBufStr.end());

    const char termCmd[] = "terminate";
    size_t sizeOfCmd = sizeof(termCmd) - 1;

    return inBuffer.size() < sizeOfCmd || strncmp((const char *)&inBuffer[0], termCmd, sizeOfCmd);
}
