#pragma once
#include <functional>
#include <string>

// Add a parameter!
void request(const std::string& url, std::function<void(long)> callback);