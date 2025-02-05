#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "background.h"
#include "bot.h"
#include "readFile.h"

int main(int argc, char** argv)
{
    const std::vector<std::string> envData = readFile("../.env");

    std::thread t1(botThread, envData);
    std::thread t2(backgroundThread, envData);

    t1.join();
    t2.join();
}