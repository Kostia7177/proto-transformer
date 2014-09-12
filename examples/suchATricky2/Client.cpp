#include <ProtoTransformer/Client.hpp>
#include "Proto.hpp"
#include <string>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

using namespace boost::program_options;

int usage(
    int ret,
    char *argv0,
    options_description &opts)
{
    (ret ? std::cerr : std::cout) << "usage: " << argv0 << " <params>" << std::endl
                                  << opts << std::endl
                                  << "Command format: [pause/]<string>" << std::endl
                                  << "  string format: <cmd>,<num1>,<num2>,<num3>,<num4>,...<numN>" << std::endl
                                  << "    available commands:" << std::endl
                                  << "    - sort" << std::endl
                                  << "    - sum" << std::endl
                                  << "    - log (not returns anything)" << std::endl
                                  << "    - terminate" << std::endl
                                  << "    - <something else> (simple echo)" << std::endl;
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

        Client<ProtoWithAnyHdr> client(addr, port);

        std::string requestString;
        while (std::cout << "request '[pause/]string'> ", std::cin >> requestString)
        {
            AnyHdr hdr;
            bzero(&hdr, sizeof(hdr));
            uint32_t hdrOpt = numOfOpts;

            std::string::size_type endOfSubstr = requestString.find('/');
            std::string data;
            if (endOfSubstr == std::string::npos)
            {
                data = requestString;
                hdr.set<pauseField>(0);
            }
            else
            {
                data = std::string(requestString, endOfSubstr + 1);
                try { hdr.set<pauseField>(stoi(std::string(requestString, 0, endOfSubstr))); }
                catch (const std::invalid_argument &exc)
                {
                    std::cerr << "Cannot understand what pause is to be (" << exc.what() << "); " << std::endl;
                    continue;
                }
            }
            boost::tokenizer<> dataTokenized(data);
            if (dataTokenized.begin() != dataTokenized.end())
            {
                std::string elem = *dataTokenized.begin();
                if (elem == "help")
                {
                    usage(0, argv[0], inOpts);
                    continue;
                }

                hdrOpt = elem == "sum" ? sumNumbers
                            : (elem == "sort" ? sortNumbers
                                : (elem == "terminate" ? terminateSession
                                    : (elem == "log" ? logRequest
                                        : echo)));
                hdr.set<optField>(hdrOpt);
            }

            RequestData requestBuffer;
            for (const std::string &elem : dataTokenized)
            {
                try { requestBuffer.push_back(stoi(elem)); }
                catch (const std::invalid_argument &) {}
            }
            if (hdrOpt == logRequest || hdrOpt == terminateSession)
            {
                client.send(hdr, requestBuffer);
            }
            else
            {
                AnswerData answerData = client.request(hdr, requestBuffer);
                std::cout << "Answer: " << std::endl;
                for (const Answer &answer : answerData)
                {
                    std::cout << answer << std::endl;
                }
            }
        }
    }
    catch(std::runtime_error &exc)
    {
        std::cerr << "ERROR: " << exc.what() << "; " << std::endl;
        return 1;
    }

    return 0;
}
