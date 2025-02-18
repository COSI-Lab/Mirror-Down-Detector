#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#include <ping.h>

std::pair<bool, std::string> ping(std::string url)
{
    // command string made of url
    std::string command = std::string("ping ") + url + " -c 3 -W 1 >ping.txt";
    // run the command
    std::system(command.c_str());
    // read the command output from the file into string
    std::ifstream      pingFile("ping.txt");
    std::ostringstream ss;
    ss << pingFile.rdbuf();
    std::string pingStr = ss.str();

    // parse the ping status
    std::regex expression("(3 packets transmitted, 3 received?)");
    std::regex expression2("(3 packets transmitted, 2 received?)");

    bool up = std::regex_search(pingStr, expression)
           || std::regex_search(pingStr, expression2);

    // return a status and the ping output
    std::pair<bool, std::string> pingObj(up, pingStr);
    return pingObj;
}
