#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "background.h"
#include "bot.h"
#include "readFile.h"

int main()
{
    const std::vector<std::string> envData = readFile("/down-detector/resources/.env");
    
    std::thread t1(botThread, envData);
    std::thread t2(backgroundThread, envData);

    t1.join();
    t2.join();
}