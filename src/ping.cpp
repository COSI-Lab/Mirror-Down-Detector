// Header Being Defined
#include <mirror/down_detector/ping.hpp>

// System Headers
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

// Standard Library Headers
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

constexpr time_t  PING_TIMEOUT_SECONDS = 10;
constexpr ssize_t PING_BUFFER          = 1024;

/**
 * @return `true` if the URL or IP responds to ping requests, and `false`
 * otherwise.
 */
auto ping(const std::string& url) -> std::pair<bool, std::string>
{
    // Create pipe for IPC
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    /* Child process */
    if (pid == 0)
    {
        // Using strdup() to avoid const casting
        char* argv[] = { strdup("/bin/ping"),
                         strdup(url.c_str()),
                         strdup("-c"),
                         strdup("3"),   // 3 times
                         strdup("-W"),
                         strdup("1"),   // 1s timeout
                         strdup("-i"),
                         strdup("0.1"), // 0.1s interval
                         NULL };

        close(fd[0]);                   // don't output to console
        dup2(fd[1], 1);                 // Reassign ping's stdout to our pipe

        execve("/bin/ping", argv, NULL);
        perror("execve(2)");
        return std::make_pair(false, "execve(2) error " + errno);
    }

    if (pid == -1)
    {
        return std::make_pair(false, "Error spawning child.");
    }

    // Read ping pipe into buffer
    char* pingRes = new char[PING_BUFFER] {}; // 1024 bytes ought to be enough?

    int flags = fcntl(fd[0], F_GETFL);
    int rc    = fcntl(fd[0], F_SETFL, O_NONBLOCK | flags);

    time_t end        = time(NULL) + PING_TIMEOUT_SECONDS;
    int    pingResPtr = 0;

    int childStatus;
    while (time(NULL) < end)
    {
        ssize_t bytesRead
            = read(fd[0], pingRes + pingResPtr, PING_BUFFER - pingResPtr);
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
        if (waitpid(pid, &childStatus, WNOHANG) == pid)
        {
            pid = 0; // signal to the outside of the loop that we reaped
            break;
        }

        sleep(0.5); // lets not hog the cpu
    }

    if (pid != 0)
    { // we need to clean up the zombie
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }

    close(fd[0]);
    close(fd[1]);
    std::string pingStr { pingRes };
    delete pingRes;

    std::regex expression("\\s100% packet loss");
    bool       up = !std::regex_search(pingStr, expression);

    // return a status and the ping output
    std::pair<bool, std::string> pingObj(up, pingStr);
    return pingObj;
}
