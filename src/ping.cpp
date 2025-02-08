#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <cstring>
#include <string>
#include <utility>
#include <ctime>

#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <fcntl.h>

#include "ping.h"

#define PING_TIMEOUT_SECONDS 10

/**
 * @return `true` if the URL or IP responds to ping requests, and `false`
 * otherwise.
 */
std::pair<bool, std::string> ping(std::string url)
{
    // Create pipe for IPC
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    /* Child process */
    if(pid == 0)
    {
        // The compiler gets pissy if we don't explicitly cast to char*
        char* argv[] = {(char*)"/bin/ping",
                         (char*)url.c_str(),
                         (char*)"-c", (char*)"3", // 3 times
                         (char*)"-W", (char*)"1", // 1s timeout
                         (char*)"-i", (char*)"0.1", NULL}; // 0.1s interval
        close(fd[0]); // don't output to console
        dup2(fd[1], 1); // Reassign ping's stdout to our pipe
        execve("/bin/ping", argv, NULL);
        perror("execve(2)");
        return std::make_pair(false, "execve(2) error " + errno);
    }

    if(pid == -1)
    {
        return std::make_pair(false, "Error spawning child.");
    }
    
    // Read ping pipe into buffer
    char *pingRes = new char[1024]{}; // 1024 bytes ought to be enough?

    int flags = fcntl(fd[0], F_GETFL);
    int rc = fcntl(fd[0], F_SETFL, O_NONBLOCK | flags);

    time_t end = time(NULL) + PING_TIMEOUT_SECONDS;
    int pingResPtr = 0;

    int childStatus;
    while(time(NULL) < end) {
        int bytesRead = read(fd[0], pingRes + pingResPtr, 1024-pingResPtr);
        if(bytesRead == 0) break; // EOF
        if(bytesRead == -1 && (errno != EAGAIN || errno != EWOULDBLOCK)) break;
        if(bytesRead > 0) pingResPtr += bytesRead;
        if(waitpid(pid, &childStatus, WNOHANG) == pid) {
            pid = 0; // signal to the outside of the loop that we reaped
            break;
        }

        sleep(0.5); // lets not hog the cpu
    }

    if(pid != 0) { // we need to clean up the zombie
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }

    std::string pingStr{pingRes};

    // TODO: change this to check for < 100% packet loss
    std::regex expression("(3 packets transmitted, 3 received?)");
    std::regex expression2("(3 packets transmitted, 2 received?)");

    bool up = std::regex_search(pingStr, expression)
           || std::regex_search(pingStr, expression2);

    // return a status and the ping output
    std::pair<bool, std::string> pingObj(up, pingStr);
    return pingObj;
}