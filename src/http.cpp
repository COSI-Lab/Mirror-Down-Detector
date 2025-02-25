// Header Being Defined
#include <mirror/down_detector/http.hpp>

// Standard Library Includes
#include <cstddef>
#include <functional>
#include <string>

// Third Party Includes
#include <curl/curl.h>
#include <curl/easy.h>
#include <spdlog/spdlog.h>

std::size_t
write_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
{
    return size * nmemb;
}

auto request(const std::string& url, const std::function<void(long)>& callback)
    -> void
{
    ::CURL* cURLHandle = ::curl_easy_init();
    if (cURLHandle == nullptr)
    {
        spdlog::error("Error initalizing cURL!");
        return;
    }

    ::CURLcode cURLStatus = CURLE_OK;

    ::curl_easy_setopt(cURLHandle, CURLOPT_URL, url.c_str());

    ::curl_easy_setopt(
        cURLHandle,
        CURLOPT_FOLLOWLOCATION, // Follow 3xx redirects
        static_cast<long>(true)
    );

    ::curl_easy_setopt(
        cURLHandle,
        CURLOPT_WRITEFUNCTION,
        write_callback
    ); // Discard output

    ::curl_easy_setopt(cURLHandle, CURLOPT_TIMEOUT, 15L);
    cURLStatus = ::curl_easy_perform(cURLHandle);
    if (cURLStatus == CURLE_OK)
    {
        long httpResponseStatus = 0;
        ::curl_easy_getinfo(
            cURLHandle,
            CURLINFO_RESPONSE_CODE,
            &httpResponseStatus
        );
        callback(httpResponseStatus);
    }
    else
    {
        callback(0);
    }

    ::curl_easy_cleanup(cURLHandle);
}
