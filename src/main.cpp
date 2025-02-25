#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <mirror/down_detector/background.hpp>
#include <mirror/down_detector/bot.hpp>
#include <mirror/down_detector/readFile.hpp>

auto main() -> int
{
    const std::vector<std::string> envData
        = readFile("/down-detector/resources/.env");

    std::thread t1(botThread, envData);
    std::thread t2(backgroundThread, envData);

    t1.join();
    t2.join();
}
