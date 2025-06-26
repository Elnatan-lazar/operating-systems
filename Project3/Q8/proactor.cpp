
#include "proactor.h"
#include <thread>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>

struct ProactorArgs {
    int listenfd;
    proactorFunc threadFunc;
};

static std::atomic<bool> running(true);

void* proactorMainLoop(void* args) {
    ProactorArgs* pArgs = static_cast<ProactorArgs*>(args);
    int listenfd = pArgs->listenfd;
    proactorFunc threadFunc = pArgs->threadFunc;
    delete pArgs;

    while (running.load()) {
        int clientfd = accept(listenfd, nullptr, nullptr);
        if (clientfd >= 0) {
            std::thread t(threadFunc, clientfd);
            t.detach();
        }
    }
    return nullptr;
}

pthread_t startProactor(int sockfd, proactorFunc threadFunc) {
    pthread_t tid;
    auto* args = new ProactorArgs{sockfd, threadFunc};
    pthread_create(&tid, nullptr, proactorMainLoop, args);
    return tid;
}

int stopProactor(pthread_t tid) {
    running = false;
    return pthread_cancel(tid);
}
