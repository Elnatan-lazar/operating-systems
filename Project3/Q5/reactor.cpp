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
    std::vector<int> fds;
    std::unordered_map<int, reactorFunc> handlers;
    std::atomic<bool> running;
    std::thread loopThread;
    std::mutex mtx;
};

static Reactor* globalReactorInstance = nullptr;

void* startReactor() {
    if (globalReactorInstance == nullptr) {
        globalReactorInstance = new Reactor();
        globalReactorInstance->running = true;

        globalReactorInstance->loopThread = std::thread([](){
            while (globalReactorInstance->running) {
                fd_set readfds;
                FD_ZERO(&readfds);
                int maxfd = -1;
                std::vector<int> local_fds;
                std::unordered_map<int, reactorFunc> local_handlers;

                {
                    std::lock_guard<std::mutex> lk(globalReactorInstance->mtx);
                    local_fds = globalReactorInstance->fds;
                    local_handlers = globalReactorInstance->handlers;
                }

                for (int fd : local_fds) {
                    FD_SET(fd, &readfds);
                    if (fd > maxfd) maxfd = fd;
                }

                if (maxfd == -1) {
                    usleep(100000); // 0.1s sleep if no fds
                    continue;
                }

                struct timeval tv = {1, 0}; // 1s timeout
                int ret = select(maxfd + 1, &readfds, nullptr, nullptr, &tv);
                if (ret > 0) {
                    for (int fd : local_fds) {
                        if (FD_ISSET(fd, &readfds)) {
                            auto it = local_handlers.find(fd);
                            if (it != local_handlers.end() && it->second) {
                                it->second(fd);
                            }
                        }
                    }
                } else if (ret == -1) {
                    perror("select error");
                }
            }
        });
    }
    return globalReactorInstance;
}

int addFdToReactor(void* reactor, int fd, reactorFunc func) {
    Reactor* r = static_cast<Reactor*>(reactor);
    std::lock_guard<std::mutex> lk(r->mtx);
    if (std::find(r->fds.begin(), r->fds.end(), fd) == r->fds.end()) {
        r->fds.push_back(fd);
        r->handlers[fd] = func;
        return 0;
    }
    return -1;
}

int removeFdFromReactor(void* reactor, int fd) {
    Reactor* r = static_cast<Reactor*>(reactor);
    std::lock_guard<std::mutex> lk(r->mtx);
    r->fds.erase(std::remove(r->fds.begin(), r->fds.end(), fd), r->fds.end());
    r->handlers.erase(fd);
    return 0;
}

int stopReactor(void* reactor) {
    Reactor* r = static_cast<Reactor*>(reactor);
    r->running = false;
    if (r->loopThread.joinable())
        r->loopThread.join();
    delete r;
    globalReactorInstance = nullptr;
    return 0;
}
