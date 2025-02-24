#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <mirror/down_detector/background.h>
#include <mirror/down_detector/bot.h>
#include <mirror/down_detector/readFile.h>

int main()
{
    const std::vector<std::string> envData = readFile("/down-detector/resources/.env");
    
    std::thread t1(botThread, envData);
    std::thread t2(backgroundThread, envData);

    t1.join();
    t2.join();
}
