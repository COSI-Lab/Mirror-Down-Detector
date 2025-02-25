#pragma once

// Standard Library Includes
#include <string>
#include <utility>

auto ping(const std::string& url) -> std::pair<bool, std::string>;
