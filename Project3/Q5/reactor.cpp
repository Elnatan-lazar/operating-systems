#include "reactor.h"
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <sys/select.h>
#include <unistd.h>
#include <algorithm>

struct Reactor {
    std::vector<int> fds;       // list of file descriptors
    std::unordered_map<int,reactorFunc> handlers; // map fd→callback
    std::atomic<bool> running;  // loop continues while true
    std::thread loopThread;
    std::mutex mtx;      // protects fds+handlers
};

void *startReactor() {
    auto *r = new Reactor();
    r->running = true;

    // spawn the event loop in background
    r->loopThread = std::thread([r](){
        while (r->running) {
            fd_set readfds;
            FD_ZERO(&readfds);

            int maxfd = -1;
            { // lock only while setting fd_set
                std::lock_guard<std::mutex> lk(r->mtx);
                for (int fd : r->fds) {
                    FD_SET(fd, &readfds);
                    if (fd > maxfd) maxfd = fd;
                }
            }

            struct timeval tv{1, 0}; // timeout 1s
            int ret = select(maxfd+1, &readfds, nullptr, nullptr, &tv);
            if (ret > 0) {
                std::lock_guard<std::mutex> lk(r->mtx);
                for (int fd : r->fds) {
                    if (FD_ISSET(fd, &readfds)) {
                        // call the registered callback
                        reactorFunc fn = r->handlers[fd];
                        if (fn) fn(fd);
                    }
                }
            }
        }
    });

    return r;
}

int addFdToReactor(void *reactor, int fd, reactorFunc func) {
    Reactor *r = static_cast<Reactor*>(reactor);
    std::lock_guard<std::mutex> lk(r->mtx);
    r->fds.push_back(fd);
    r->handlers[fd] = func;
    return 0;
}

int removeFdFromReactor(void *reactor, int fd) {
    Reactor *r = static_cast<Reactor*>(reactor);
    std::lock_guard<std::mutex> lk(r->mtx);
    r->fds.erase(std::remove(r->fds.begin(), r->fds.end(), fd), r->fds.end());
    r->handlers.erase(fd);
    return 0;
}

int stopReactor(void *reactor) {
    Reactor *r = static_cast<Reactor*>(reactor);
    r->running = false;
    if (r->loopThread.joinable())
        r->loopThread.join();
    delete r;
    return 0;
}
