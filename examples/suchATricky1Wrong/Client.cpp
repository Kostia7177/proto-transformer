#include <ProtoTransformer/Client.hpp>
#include <boost/program_options.hpp>
#include "Proto.hpp"

using namespace boost::program_options;

int usage(
    int ret,
    char *argv0,
    options_description &opts)
{
    (ret ? std::cerr : std::cout) << "usage: " << argv0 << " <params>" << std::endl
                                  << opts << std::endl;
    return ret;
}

using namespace ProtoTransformer;

int main(
    int argc,
    char **argv)
{
    bool helpNeeded;
    std::string addr, key;
    int port, numOfRequests;
    options_description inOpts;
    inOpts.add_options()
        ("help,h",  bool_switch(&helpNeeded),   "print this help text and exit")
        ("ip,i",    value<std::string>(&addr),  "server's ip (in dot-notation)")
        ("port,p",  value<int>(&port),          "server port")
        ("num,n",   value<int>(&numOfRequests), "number of requests untill session will terminated")
        ("key,k",   value<std::string>(&key),   "session key");

    if (argc < 3) { return usage(1, argv[0], inOpts); }
    try
    {
        variables_map vm;
        store(parse_command_line(argc, argv, inOpts), vm);
        notify(vm);
        if (helpNeeded) { return usage(0, argv[0], inOpts); }

        AnySessionHdr sessionHdr;
        uint32_t *hdrNum = getNum(sessionHdr);
        *hdrNum = htonl(numOfRequests);
        snprintf(getName(sessionHdr), getNameSize(sessionHdr), "%s", key.c_str());
        Client<ProtoWithSessionHdr> client(addr, port, sessionHdr);

        std::string requestString;
        while (std::cout << "requests left " << numOfRequests << "> ", std::cin >> requestString)
        {
            AnyAnswerHdr answerHdr;
            AnswerData answerBuffer = client.request(RequestData(requestString.begin(), requestString.end()), &answerHdr);
            answerBuffer.push_back(0);
            std::cout << "Answer : " << &answerBuffer[0] << "; " << std::endl;
            numOfRequests = *getNumPtr(answerHdr);
        }
    }
    catch (std::runtime_error &exc)
    {
        std::cerr << "ERROR: " << exc.what() << "; " << std::endl;
        return 1;
    }

    return 0;
}
