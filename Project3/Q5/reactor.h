#ifndef REACTOR_H
#define REACTOR_H

#include <cstddef>

// function pointer type called when fd is ready
typedef void *(*reactorFunc)(int fd);

// opaque Reactor type
typedef struct Reactor Reactor;

// creates and starts a Reactor; returns pointer to it
// (you must call stopReactor() to clean up)
void *startReactor();

// adds an fd to be monitored for read readiness
// and associates it with callback func
// returns 0 on success
int addFdToReactor(void *reactor, int fd, reactorFunc func);

// removes a previously added fd from the Reactor
// returns 0 on success
int removeFdFromReactor(void *reactor, int fd);

// stops the Reactor loop and frees resources
// returns 0 on success
int stopReactor(void *reactor);

#endif // REACTOR_H
