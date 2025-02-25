#pragma once

// Standard Library Includes
#include <string>
#include <vector>

auto readFile(const std::string& filename) -> std::vector<std::string>;

auto readFile2d(const std::string& filename)
    -> std::vector<std::vector<std::string>>;

auto writeFile2d(
    const std::vector<std::vector<std::string>>& inputMatrix,
    const std::string&                           filename
) -> void;

auto hasChannel(const std::string& filename, const std::string& channelId)
    -> bool;
