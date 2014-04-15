#include <ProtoTransformer/Client.hpp>
#include "Proto.hpp"
#include <boost/program_options.hpp>

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
    std::string addr;
    int port;
    options_description inOpts;
    inOpts.add_options()
        ("help,h",  bool_switch(&helpNeeded),   "print this help text and exit")
        ("ip,i",    value<std::string>(&addr),  "server's ip (in dot-notation)")
        ("port,p",  value<int>(&port),          "server port");

    if (argc < 3) { return usage(1, argv[0], inOpts); }
    try
    {
        variables_map vm;
        store(parse_command_line(argc, argv, inOpts), vm);
        notify(vm);
        if (helpNeeded) { return usage(0, argv[0], inOpts); }

        Client<ProtoWithoutAnswer> client(addr, port);
        std::string requestString;
        while (std::cout << "request 'no answer'> ", std::cin >> requestString)
        {
            RequestData requestBuffer(requestString.begin(), requestString.end());
            requestBuffer.push_back(0);
            client.request(requestBuffer);
        }
    }
    catch(std::runtime_error &exc)
    {
        std::cerr << "ERROR: '" << exc.what() << "'; " << std::endl;
        return 1;
    }

    return 0;
}
