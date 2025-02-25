// Header Being Defined
#include <mirror/down_detector/ping.hpp>

// System Headers
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Standard Library Headers
#include <array>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

constexpr std::time_t PING_TIMEOUT_SECONDS = 10;
constexpr std::size_t PING_BUFFER          = 1024;

/**
 * @return `true` if the URL or IP responds to ping requests, and `false`
 * otherwise.
 */
auto ping(const std::string& url) -> std::pair<bool, std::string>
{
    // Create pipe for IPC
    std::array<int, 2> pipes = { -1, -1 };
    ::pipe(pipes.data());

    ::pid_t pid = ::fork();

    /* Child process */
    if (pid == 0)
    {
        // Using strdup() to avoid const casting
        std::array<char*, 9> argv = { ::strdup("/bin/ping"),
                                      ::strdup(url.c_str()),
                                      ::strdup("-c"),
                                      ::strdup("3"),   // 3 times
                                      ::strdup("-W"),
                                      ::strdup("1"),   // 1s timeout
                                      ::strdup("-i"),
                                      ::strdup("0.1"), // 0.1s interval
                                      NULL };

        ::close(pipes.at(0)); // don't output to console
        ::dup2(
            pipes.at(1),
            STDOUT_FILENO
        ); // Reassign ping's stdout to our pipe

        ::execve("/bin/ping", argv.data(), nullptr);
        ::perror("execve(2)");
        return std::make_pair(
            false,
            "execve(2) error " + std::to_string(errno)
        );
    }

    if (pid == -1)
    {
        return std::make_pair(false, "Error spawning child.");
    }

    // Read ping pipe into buffer
    std::string pingRes; // 1024 bytes ought to be enough?
    pingRes.resize(PING_BUFFER);

    const int fileDescriptorFlags = ::fcntl(pipes.at(0), F_GETFL);
    const int status
        = ::fcntl(pipes.at(0), F_SETFL, O_NONBLOCK | fileDescriptorFlags);

    if (status != 0)
    {
        // Failed to update flags on pipe
    }

    const std::time_t end        = std::time(nullptr) + PING_TIMEOUT_SECONDS;
    int               pingResPtr = 0;

    int childStatus;
    while (std::time(nullptr) < end)
    {
        auto bytesRead = ::read(
            pipes.at(0),
            pingRes + pingResPtr,
            PING_BUFFER - pingResPtr
        );
        if (bytesRead == 0)
        {
            break; // EOF
        }
        if (bytesRead == -1 && (errno != EAGAIN || errno != EWOULDBLOCK))
        {
            break;
        }
        if (bytesRead > 0)
        {
            pingResPtr += bytesRead;
        }
        if (::waitpid(pid, &childStatus, WNOHANG) == pid)
        {
            pid = 0; // signal to the outside of the loop that we reaped
            break;
        }

        ::sleep(1); // lets not hog the cpu
    }

    if (pid != 0)
    { // we need to clean up the zombie
        ::kill(pid, SIGKILL);
        ::waitpid(pid, NULL, 0);
    }

    ::close(pipes.at(0));
    ::close(pipes.at(1));

    constexpr std::regex expression("\\s100% packet loss");
    const bool           mirrorIsUp = !std::regex_search(pingStr, expression);

    // return a status and the ping output
    std::pair<bool, std::string> pingObj(mirrorIsUp, pingStr);
    return pingObj;
}
