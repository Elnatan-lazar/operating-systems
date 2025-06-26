
#ifndef PROACTOR_H
#define PROACTOR_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*proactorFunc)(int sockfd);

// Starts a proactor: listens on sockfd, spawns thread for each accepted client.
pthread_t startProactor(int sockfd, proactorFunc threadFunc);

// Stops the proactor thread
int stopProactor(pthread_t tid);

#ifdef __cplusplus
}
#endif

#endif // PROACTOR_H
