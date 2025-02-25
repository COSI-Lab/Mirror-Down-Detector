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
std::vector<std::string> splitString(const std::string& str)
{
    std::vector<std::string> output;
    std::size_t              startOfToken = 0;
    std::size_t              endOfToken   = str.find(' ');
    while (endOfToken != -1)
    {
        output.push_back(str.substr(startOfToken, endOfToken - startOfToken));
        startOfToken = endOfToken + 1;
        endOfToken   = str.find(' ', startOfToken);
    }
    // get the last item in list
    output.push_back(str.substr(startOfToken, endOfToken - startOfToken));
    return output;
}

auto readFile(const std::string& filename) -> std::vector<std::string>
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
        ::exit(EXIT_FAILURE);
    }
    file.close();

    return output;
}

auto readFile2d(const std::string& filename)
    -> std::vector<std::vector<std::string>>
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
        ::exit(EXIT_FAILURE);
    }
    file.close();
    return output;
}

auto writeFile2d(
    const std::vector<std::vector<std::string>>& inputMatrix,
    const std::string&                           filename
) -> void
{
    std::ofstream channelFile;
    channelFile.open(filename);
    for (std::size_t idx = 0; idx < inputMatrix.size(); idx++)
    {
        for (std::size_t jdx = 0; jdx < inputMatrix.at(idx).size(); jdx++)
        {
            channelFile << inputMatrix.at(idx).at(jdx) << " ";
        }
        channelFile << '\n';
    }
    channelFile.close();
}

auto hasChannel(const std::string& filename, const std::string& channelId)
    -> bool
{
    std::vector<std::vector<std::string>> channels_roles = readFile2d(filename);

    for (int i = 0; i < channels_roles.size(); i++)
    {
        if (channelId == channels_roles.at(i).at(0))
        {
            return true;
        }
    }

    return false;
}
