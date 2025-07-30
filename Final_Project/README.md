````markdown
# Final_Project

This project implements a simple undirected graph data structure and finds an Eulerian circuit (if one exists). It also supports random graph generation, profiling, and a TCP server interface.

## File Structure

- **Graph.h / Graph.cpp**
- **Algorithms.h / Algorithms.cpp**
- **main.cpp**
- **random_graph.cpp**
- **server_Q6.cpp** — TCP server handling `CREATE`/`CHECK`/`RESET`/`QUIT`
- **Makefile**
- **README.md**

## 1. Build

```bash
make
```
````

This builds three binaries:

- `graph_app` — fixed-input mode
- `random_app` — random-graph generation
- `server_app` — TCP server on port 8080

## 2. Run fixed-input mode

```bash
make run
# runs: ./graph_app $(RUN_ARGS)
```

## 3. Run random-graph mode

```bash
make run_random
# runs: ./random_app $(RUN_RANDOM_ARGS)
```

## 4. Code Coverage

```bash
make coverage
./graph_app_cov 3 0 1 1 2 2 0
gcov *.cpp
```

## 5. gprof Profiling

```bash
make profile
./graph_app_prof 3 0 1 1 2 2 0
gprof graph_app_prof gmon.out > gprof_report.txt
```

## 6. Valgrind Memcheck

```bash
make memcheck
# output in memcheck_report.txt
```

## 7. Valgrind Callgrind

```bash
make callgrind
# output in callgrind_report.txt
```

## 8. Server (Stage 5)

### Build & Run

```bash
make server_app
make run_server
```

The server listens on port **8080**.

### How to Connect

Use `netcat` or `telnet`:

```bash
nc localhost 8080
# or
telnet localhost 8080
```

#### Protocol

After connection, you’ll see:

```
WELCOME to Euler-Server
Available commands:
  CREATE <num_vertices> <u1> <v1> [<u2> <v2> ...]
  CHECK
  RESET
  QUIT
```

- **CREATE**: build a new graph

  ```
  CREATE 4 0 1 1 2 2 3 3 0
  ```

- **CHECK**: returns an Eulerian circuit or `NO_EULERIAN_CIRCUIT`
- **RESET**: discard current graph
- **QUIT**: close connection

On bad input you’ll get an error plus a usage hint.

```

```
