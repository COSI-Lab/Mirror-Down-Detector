// Header Being Defined
#include <mirror/down_detector/readFile.hpp>

// Standard Library Includes
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/**
 * I have a feeling that this exists somewhere in the STL
 */
std::vector<std::string> splitString(std::string str)
{
    std::vector<std::string> output;
    int                      start = 0;
    int                      end   = str.find(" ");
    while (end != -1)
    {
        output.push_back(str.substr(start, end - start));
        start = end + 1;
        end   = str.find(" ", start);
    }
    // get the last item in list
    output.push_back(str.substr(start, end - start));
    return output;
}

std::vector<std::string> readFile(std::string filename)
{
    std::vector<std::string> output {};
    std::string              line;
    std::ifstream            file(filename);

    while (getline(file, line))
    {
        output.push_back(line);
    }
    if (!file.is_open())
    {
        std::cout << "I/O error while opening " << filename << ".\n";
        exit(EXIT_FAILURE);
    }
    file.close();

    return output;
}

std::vector<std::vector<std::string>> readFile2d(std::string filename)
{
    std::vector<std::vector<std::string>> output {};
    std::string                           line;
    std::ifstream                         file(filename);
    while (getline(file, line))
    {
        output.push_back(splitString(line));
    }
    if (!file.is_open())
    {
        std::cout << "I/O error while opening " << filename << ".\n";
        exit(EXIT_FAILURE);
    }
    file.close();
    return output;
}

void writeFile2d(
    std::vector<std::vector<std::string>> inputMatrix,
    std::string                           filename
)
{
    std::ofstream channelFile;
    channelFile.open(filename);
    for (int i = 0; i < inputMatrix.size(); i++)
    {
        for (int j = 0; j < inputMatrix[i].size(); j++)
        {
            channelFile << inputMatrix[i][j] << " ";
        }
        channelFile << std::endl;
    }
    channelFile.close();
}

bool hasChannel(std::string filename, std::string channelId)
{
    std::vector<std::vector<std::string>> channels_roles = readFile2d(filename);

    for (int i = 0; i < channels_roles.size(); i++)
    {
        if (channelId == channels_roles[i][0])
        {
            return true;
        }
    }

    return false;
}
