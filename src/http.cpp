// Header Being Defined
#include <mirror/down_detector/http.hpp>

// Standard Library Includes
#include <functional>
#include <future>
#include <iostream>
#include <string>

// Third Party Includes
#include <curl/curl.h>

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    return size * nmemb;
}

auto request(const std::string& url, std::function<void(long)> callback) -> void
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Error initializing cURL.\n";
        return;
    }

    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow 3xx redirects
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        write_callback
    ); // Discard output
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    res = curl_easy_perform(curl);
    if (res == CURLE_OK)
    {
        long resp;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp);
        callback(resp);
    }
    else
    {
        callback(0);
    }
    curl_easy_cleanup(curl);
}
