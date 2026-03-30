# Operating Systems Projects — Ariel University

A collection of four C/C++ projects developed as part of the Operating Systems course at Ariel University. Each project incrementally builds knowledge of core OS concepts, progressing from signal handling and process debugging through networked multi-client servers, thread synchronization, and graph algorithms over TCP.

---

## Repository Structure

```
operating-systems/
├── Project1/        # Signals, processes, shared libraries, debugging & profiling
├── Project2/        # Sockets, threads, synchronization, shared memory
├── Project3/        # Networked convex hull server: reactor & proactor patterns
└── Final_Project/   # Eulerian circuit detection over TCP
```

---

## Projects

### Project 1 — Processes, Signals, Debugging & Profiling (C)

Seven sub-exercises exploring low-level C programming and Linux tooling.

| Exercise | Topic |
|----------|-------|
| Q1 | Triggering OS signals: stack overflow, division by zero, invalid memory access |
| Q2 | Mandelbrot set calculation — complex number math with shared library (`libmandelbrot.so`) |
| Q3 | Dynamic shared library loading (`dlopen`/`dlsym`) |
| Q4 | Dijkstra shortest-path algorithm with `gcov` code coverage analysis |
| Q5 | Maximum sub-array sum with `gprof` performance profiling |
| Q6 | Inter-process communication via UNIX signals (`SIGUSR1`/`SIGUSR2`) — bit-level messaging between Sender and Receiver processes |
| Q7 | Phone-book application using file I/O — add and search entries |

**Tools:** `gcc`, `gcov`, `gprof`, `valgrind`, `make`

---

### Project 2 — Sockets, Threads & Synchronization (C++)

Six stages building a networked Atom Warehouse / Drinks Bar simulation.

| Exercise | Topic |
|----------|-------|
| Q1 | TCP atom warehouse server using `select()` — accepts `ADD <ATOM> <N>` commands |
| Q2 | UDP molecule requester — clients request molecules assembled from atoms |
| Q3 | Drinks bar combining TCP + UDP: server handles both protocols simultaneously |
| Q4 | Command-line argument parsing (`getopt`), configurable TCP/UDP ports, signal-based shutdown, and timeout support |
| Q5 | Multi-supplier simulation — multiple concurrent producers send atoms/molecules |
| Q6 | Shared memory (`mmap`) + file locking (`flock`) for inter-process inventory management; UNIX domain sockets |

**Tools:** `g++`, POSIX sockets, `pthread`, `select()`, `mmap`, `flock`

---

### Project 3 — Convex Hull TCP Server: Reactor & Proactor (C++)

Ten stages implementing a multi-client TCP server that computes convex hull area for 2D point sets.

| Exercise | Topic |
|----------|-------|
| Q1 | CLI convex hull — reads points from stdin, outputs hull and area |
| Q2 | Convex hull from file input with profiling |
| Q3 | Interactive command interface: `Newgraph`, `Newpoint`, `Removepoint`, `CH` |
| Q4 | Multi-client TCP server with shared graph state |
| Q5 | Reactor library — `select()`-based event loop (`startReactor`, `addFdToReactor`, `stopReactor`) |
| Q6 | Server rebuilt on top of the Reactor library |
| Q7 | Thread-per-client server using `std::thread` + mutex-protected shared graph |
| Q8 | Proactor library — spawns a thread per accepted connection (`startProactor`, `stopProactor`) |
| Q9 | Server rebuilt on top of the Proactor library |
| Q10 | Producer-consumer area watcher using `pthread_cond_t` — alerts when hull area crosses 100 units |

**Tools:** `g++ -std=c++11`, `pthread`, TCP sockets, `select()`, condition variables

---

### Final Project — Eulerian Circuit Detection over TCP (C++)

A graph server that detects Eulerian circuits using Hierholzer's algorithm, with optional code coverage, profiling, and memory analysis.

**Components:**
- `Graph.h / Graph.cpp` — undirected graph data structure
- `Algorithms.h / Algorithms.cpp` — Eulerian circuit algorithm
- `main.cpp` — fixed-input CLI mode
- `random_graph.cpp` — random graph generator
- `server_Q6-Q9.cpp` / `client_Q6-Q9.cpp` — TCP server on port 8080 accepting `CREATE`, `CHECK`, `RESET`, `QUIT` commands

**Protocol (connect via `nc localhost 8080`):**
```
CREATE <num_vertices> <u1> <v1> [<u2> <v2> ...]
CHECK
RESET
QUIT
```

**Tools:** `g++`, `gcov`, `gprof`, `valgrind` (memcheck + callgrind), `make`

---

## Technologies

- **Languages:** C, C++11
- **OS:** Linux (POSIX)
- **Build system:** GNU Make
- **Concurrency:** `pthread`, `std::thread`, `std::mutex`, `pthread_cond_t`
- **Networking:** POSIX TCP/UDP sockets, `select()`, UNIX domain sockets
- **IPC:** Signals, shared memory (`mmap`), file locking (`flock`)
- **Profiling & debugging:** `gcov`, `gprof`, `valgrind` (memcheck, callgrind)
- **Topics:** Reactor pattern, Proactor pattern, Producer-Consumer, graph algorithms

---

## Building & Running

Each project and sub-exercise has its own `Makefile`. General workflow:

```bash
# Enter a sub-exercise directory, e.g.:
cd Project2/Q4

# Build
make

# Run server (example)
./drinks_bar -T 5555 -U 6666

# Connect from another terminal
nc localhost 5555

# Clean build artifacts
make clean
```

For the Final Project:
```bash
cd Final_Project
make               # builds graph_app, random_app, server_app
make run_server    # starts TCP server on port 8080
nc localhost 8080
```

---

## Course Context

**Course:** Operating Systems
**Institution:** Ariel University
**Authors:** Elnatan & Noa
**Year:** 2024

