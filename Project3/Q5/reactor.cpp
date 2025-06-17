#include "reactor.h"
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <sys/select.h>
#include <unistd.h>
#include <algorithm> // Required for std::remove

struct Reactor {
    std::vector<int> fds;       // list of file descriptors
    std::unordered_map<int,reactorFunc> handlers; // map fd→callback
    std::atomic<bool> running;  // loop continues while true
    std::thread loopThread;
    std::mutex mtx;      // protects fds+handlers
};

// Global pointer to the single Reactor instance.
// This is a common pattern when a singleton-like behavior is desired for the reactor.
// It allows startReactor() to return a pointer to the *same* instance every time it's called.
static Reactor* globalReactorInstance = nullptr;

void *startReactor() {
    // If the reactor instance doesn't exist, create it.
    if (globalReactorInstance == nullptr) {
        globalReactorInstance = new Reactor();
        globalReactorInstance->running = true;

        // Spawn the event loop in background
        globalReactorInstance->loopThread = std::thread([](){
            while (globalReactorInstance->running) {
                fd_set readfds;
                FD_ZERO(&readfds);

                int maxfd = -1;
                std::vector<int> current_fds_copy; // Use a copy for iteration

                { // Lock only while setting fd_set
                    std::lock_guard<std::mutex> lk(globalReactorInstance->mtx);
                    // Populate the fd_set and find maxfd from the current list of fds
                    for (int fd : globalReactorInstance->fds) {
                        FD_SET(fd, &readfds);
                        if (fd > maxfd) maxfd = fd;
                    }
                    current_fds_copy = globalReactorInstance->fds; // Make a copy for safe iteration
                }

                struct timeval tv;
                tv.tv_sec = 1; tv.tv_usec = 0; // Timeout 1s
                // Use a temporary timeval as select modifies it
                struct timeval *timeout_ptr = &tv;

                // Handle maxfd == -1 when no fds are registered
                if (maxfd == -1) {
                    // No fds to monitor, sleep for the timeout period
                    usleep(tv.tv_sec * 1000000 + tv.tv_usec);
                    continue; // Continue to next loop iteration
                }

                int ret = select(maxfd+1, &readfds, nullptr, nullptr, timeout_ptr);
                if (ret > 0) {
                    std::lock_guard<std::mutex> lk(globalReactorInstance->mtx);
                    // Iterate over the copy of fds because handlers might remove fds during iteration
                    for (int fd : current_fds_copy) {
                        if (FD_ISSET(fd, &readfds)) {
                            // Check if the fd is still in the handlers map before calling,
                            // as it might have been removed by a previous handler in this loop (e.g., EXIT command)
                            auto it = globalReactorInstance->handlers.find(fd);
                            if (it != globalReactorInstance->handlers.end()) {
                                reactorFunc fn = it->second;
                                if (fn) fn(fd);
                            }
                        }
                    }
                } else if (ret == -1) {
                    // Error in select
                    perror("select error");
                    // Optionally break or handle the error
                }
                // If ret == 0, it means timeout, no fds were ready, loop continues.
            }
        });
    }
    return globalReactorInstance;
}

int addFdToReactor(void *reactor, int fd, reactorFunc func) {
    Reactor *r = static_cast<Reactor*>(reactor);
    std::lock_guard<std::mutex> lk(r->mtx);
    // Check if the fd already exists to prevent duplicates and ensure single handler
    if (std::find(r->fds.begin(), r->fds.end(), fd) == r->fds.end()) {
        r->fds.push_back(fd);
        r->handlers[fd] = func;
        return 0; // Success
    }
    return -1; // FD already exists
}

int removeFdFromReactor(void *reactor, int fd) {
    Reactor *r = static_cast<Reactor*>(reactor);
    std::lock_guard<std::mutex> lk(r->mtx);
    // Remove from fds vector
    // std::remove reorders elements, then erase removes them
    r->fds.erase(std::remove(r->fds.begin(), r->fds.end(), fd), r->fds.end());
    // Remove from handlers map
    r->handlers.erase(fd);
    return 0; // Success
}

int stopReactor(void *reactor) {
    Reactor *r = static_cast<Reactor*>(reactor);
    r->running = false;
    if (r->loopThread.joinable())
        r->loopThread.join();
    delete r;
    globalReactorInstance = nullptr; // Reset the global pointer
    return 0;
}