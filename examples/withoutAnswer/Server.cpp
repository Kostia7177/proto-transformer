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

int doSomething(const RequestData &);

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

    Server<ProtoWithoutAnswer>(port, doSomething);
    return 0;
}

int doSomething(const RequestData &inBuffer)
{
    std::cout << "in buffer: '" << &inBuffer[0] << "'; " << std::endl;
    struct timespec pause = { 1, 0 };
    nanosleep(&pause, 0);
    return 1;
}
