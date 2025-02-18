#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <background.hpp>
#include <bot.hpp>
#include <readFile.hpp>

int main()
{
    // read env file
    const std::vector<std::string> envData = readFile("../.env");

    std::thread t1(botThread, envData);
    std::thread t2(backgroundThread, envData);

    t1.join();
    t2.join();
}
