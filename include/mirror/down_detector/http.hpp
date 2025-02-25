#pragma once
// Standard Library Includes
#include <functional>
#include <string>

// Add a parameter!
auto request(const std::string& url, const std::function<void(long)>& callback)
    -> void;
