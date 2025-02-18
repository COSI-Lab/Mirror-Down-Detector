#pragma once

// Standard Library Includes
#include <string>
#include <vector>

std::vector<std::string> readFile(std::string filename);

std::vector<std::vector<std::string>> readFile2d(std::string filename);

void writeFile2d(
    std::vector<std::vector<std::string>> inputMatrix,
    std::string                           filename
);
