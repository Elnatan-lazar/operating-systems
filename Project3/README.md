# 🧠 Operating Systems Project 3 — Convex Hull Server

This project implements a **multi-client, multi-threaded TCP server** that performs **Convex Hull** calculations on 2D points. Over 10 incremental stages, it explores fundamental OS programming concepts including:

- Sockets
- `select()`-based Reactors
- Multithreading
- Mutex and condition variables
- Thread-per-client servers (Proactor Pattern)
- Producer-Consumer design

---

## 📁 Folder Structure

```
Project3/
├── Q1/         # Basic CLI convex hull
├── Q2/         # Convex hull from input file
├── Q3/         # Interactive commands via stdin
├── Q4/         # Multi-client TCP server
├── Q5/         # Reactor library (select)
├── Q6/         # Server using reactor
├── Q7/         # Thread-per-client server
├── Q8/         # Proactor library
├── Q9/         # Server with proactor
├── Q10/        # Area-watcher via condition variable
```

---

## ✅ Stage Descriptions

### 🔹 Q1 – Convex Hull CLI

**What:** Reads points from stdin, computes convex hull and prints area.  
**Run:**  
```bash
g++ -std=c++11 -o ch ch.cpp
./ch
```

---

### 🔹 Q2 – Convex Hull from File

**What:** Similar to Q1, but accepts a file with 2D points.  
**Run:**  
```bash
g++ -std=c++11 -o ch_file ch_file.cpp
./ch_file input.txt
```

---

### 🔹 Q3 – Interactive Commands

**What:** Accepts interactive commands like:
- `Newgraph n`
- `Newpoint x,y`
- `Removepoint x,y`
- `CH`

**Run:**  
```bash
g++ -std=c++11 -o ch_interactive ch_interactive.cpp
./ch_interactive
```

---

### 🔹 Q4 – TCP Server with Shared Graph

**What:** Multi-client TCP server, shared graph between all clients.  
**Run:**  
```bash
g++ -std=c++11 -pthread -o server server.cpp
./server
```

**Connect from client:**  
```bash
nc localhost 9034
```

---

### 🔹 Q5 – Reactor Template

**What:** `reactor.h/.cpp` implements a reactor using `select()`.  
Includes:
```cpp
void* startReactor();
int addFdToReactor(void*, int, reactorFunc);
int removeFdFromReactor(void*, int);
int stopReactor(void*);
```

**Build:** Use `libreactor.a` or link `reactor.cpp`.

---

### 🔹 Q6 – Server using Reactor

**What:** TCP server that registers new clients via `addFdToReactor`.  
**Run:**  
```bash
g++ -std=c++11 -pthread -I../Q5 -o server_reactor server_reactor.cpp ../Q5/reactor.cpp
./server_reactor
```

---

### 🔹 Q7 – Thread-per-client Server

**What:** Creates a new thread for each client using `std::thread`.  
Mutex protects graph access and CH computation.  
**Run:**  
```bash
g++ -std=c++11 -pthread -o server server.cpp
./server
```

---

### 🔹 Q8 – Proactor Template

**What:** Proactor implementation that spawns a thread on each accepted client:

```cpp
pthread_t startProactor(int sockfd, proactorFunc);
int stopProactor(pthread_t tid);
```

---

### 🔹 Q9 – Server with Proactor

**What:** Server uses `startProactor()` to spawn thread-per-client.  
**Mutex** protects the shared graph. Only one client may modify it at a time.

**Run:**
```bash
g++ -std=c++11 -pthread -I../Q8 -o server server.cpp ../Q8/proactor.cpp
./server
```

---

### 🔹 Q10 – Producer-Consumer via Condition Variable

**What:** Adds a thread that watches convex hull area using `pthread_cond_t`.

**When area >= 100:**
```
At Least 100 units belongs to CH
```

**When area drops below 100:**
```
At Least 100 units no longer belongs to CH
```

**Run:**
```bash
cd Q10
make
./server
```

**Sample Client:**
```bash
nc localhost 9034
```

---

## ⚙️ Example Makefile (Q10)

```makefile
CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread -I../Q8

all: server

server: server.cpp ../Q8/proactor.cpp
	$(CXX) $(CXXFLAGS) -o $@ server.cpp ../Q8/proactor.cpp

clean:
	rm -f server
```

---

## 📌 Dependencies

- C++11
- POSIX Threads (`pthread`)
- TCP Sockets
- `<select>` + `<mutex>` + `<condition_variable>`

---

## 👤 Author

> Elnatan & Noa, OS Course 2024, Ariel University