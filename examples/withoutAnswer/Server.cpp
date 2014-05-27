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

    Server<ProtoWithoutAnswer,
           UsePolicy<SessionThreadPoolIs, boost::threadpool::pool>,
           UsePolicy<ParallelRequestsPerSessionIs, Int2Type<42>>,
           UsePolicy<LoggerIs, StderrLogger>
          >(port, doSomething);
    return 0;
}

int doSomething(const RequestData &inBuffer)
{
    std::cout << "Request started at " << time(0)
              << "; with in buffer: '" << &inBuffer[0]
              << "'; at thread " << pthread_self()
              << "; " << std::endl;

    std::string request(inBuffer.begin(), inBuffer.begin() + strlen(&inBuffer[0]));
    if (request == "terminate")
    {
        std::cout << "Session termination requested at " << time(0) << "; " << std::endl;
        return 0;
    }

    std::string::size_type endOfSubstr = request.find('/');
    if (endOfSubstr != std::string::npos)
    {
        struct timespec pause = { 0, 0 };
        try { pause.tv_sec = stoi(std::string(request, 0, endOfSubstr)); }
        catch (const std::invalid_argument &){}
        nanosleep(&pause, 0);
    }
    std::cout << "Request '" << &inBuffer[0]
              << "'; processed at " << time(0) << "; " << std::endl;

    return 1;
}
