# Makefile for Final_Project — Stages 1–5 (Graph, Random, Profiling, Servers)

# ------------------------------------------------------------------------------
# Compiler and flags
# ------------------------------------------------------------------------------
CXX             := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I. -pthread

# ------------------------------------------------------------------------------
# Targets
# ------------------------------------------------------------------------------
TARGET           := graph_app
RANDOM_TARGET    := random_app
SERVER6_TARGET   := server_Q6
SERVER7_TARGET   := server_Q7
SERVER8_TARGET   := server_Q8
SERVER9_TARGET   := server_Q9

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
SRCS             := Graph.cpp Algorithms.cpp main.cpp
RANDOM_SRC       := random_graph.cpp
SERVER6_SRC      := server_Q6.cpp
SERVER7_SRC      := server_Q7.cpp
SERVER8_SRC      := server_Q8.cpp
SERVER9_SRC      := server_Q9.cpp

# ------------------------------------------------------------------------------
# Objects
# ------------------------------------------------------------------------------
OBJS             := $(SRCS:.cpp=.o)
RANDOM_OBJS      := Graph.o Algorithms.o random_graph.o
SERVER6_OBJS     := Graph.o Algorithms.o server_Q6.o
SERVER7_OBJS     := Graph.o Algorithms.o server_Q7.o
SERVER8_OBJS     := Graph.o Algorithms.o server_Q8.o
SERVER9_OBJS     := Graph.o Algorithms.o server_Q9.o

# ------------------------------------------------------------------------------
# Default arguments (override on command-line)
# ------------------------------------------------------------------------------
RUN_ARGS         ?= 3 0 1 1 2 2 0
RUN_RANDOM_ARGS  ?= -v 5 -e 7 -s 12345

.PHONY: all run run_random \
        run_server6 run_server7 run_server8 run_server9 \
        coverage profile memcheck callgrind clean clear

# ------------------------------------------------------------------------------
# 1) Default: build all executables
# ------------------------------------------------------------------------------
all: $(TARGET) $(RANDOM_TARGET) \
     $(SERVER6_TARGET) $(SERVER7_TARGET) \
     $(SERVER8_TARGET) $(SERVER9_TARGET)

# ------------------------------------------------------------------------------
# 2) Build fixed-input executable
# ------------------------------------------------------------------------------
$(TARGET): $(OBJS)
	@echo "=== Linking $(TARGET) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ">>> Built $(TARGET) <<<"

# ------------------------------------------------------------------------------
# 3) Build random-graph executable
# ------------------------------------------------------------------------------
$(RANDOM_TARGET): $(RANDOM_OBJS)
	@echo "=== Linking $(RANDOM_TARGET) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ">>> Built $(RANDOM_TARGET) <<<"

# ------------------------------------------------------------------------------
# 4) Build server Q6 executable
# ------------------------------------------------------------------------------
$(SERVER6_TARGET): $(SERVER6_OBJS)
	@echo "=== Linking $(SERVER6_TARGET) ==="
	@$(CXX) $(CXXFLAGS) -pthread -o $@ $^
	@echo ">>> Built $(SERVER6_TARGET) <<<"

# ------------------------------------------------------------------------------
# 5) Build server Q7 executable
# ------------------------------------------------------------------------------
$(SERVER7_TARGET): $(SERVER7_OBJS)
	@echo "=== Linking $(SERVER7_TARGET) ==="
	@$(CXX) $(CXXFLAGS) -pthread -o $@ $^
	@echo ">>> Built $(SERVER7_TARGET) <<<"

# ------------------------------------------------------------------------------
# 6) Build server Q8 executable
# ------------------------------------------------------------------------------
$(SERVER8_TARGET): $(SERVER8_OBJS)
	@echo "=== Linking $(SERVER8_TARGET) ==="
	@$(CXX) $(CXXFLAGS) -pthread -o $@ $^
	@echo ">>> Built $(SERVER8_TARGET) <<<"

# ------------------------------------------------------------------------------
# 7) Build server Q9 executable
# ------------------------------------------------------------------------------
$(SERVER9_TARGET): $(SERVER9_OBJS)
	@echo "=== Linking $(SERVER9_TARGET) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ">>> Built $(SERVER9_TARGET) <<<"

# ------------------------------------------------------------------------------
# 8) Compile rules
# ------------------------------------------------------------------------------
Graph.o: Graph.cpp Graph.h
	@echo "Compiling Graph.cpp → Graph.o"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

Algorithms.o: Algorithms.cpp Algorithms.h Graph.h
	@echo "Compiling Algorithms.cpp → Algorithms.o"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

main.o: main.cpp Graph.h Algorithms.h
	@echo "Compiling main.cpp → main.o"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

random_graph.o: random_graph.cpp Graph.h Algorithms.h
	@echo "Compiling random_graph.cpp → random_graph.o"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

server_Q6.o: server_Q6.cpp Graph.h Algorithms.h
	@echo "Compiling server_Q6.cpp → server_Q6.o"
	@$(CXX) $(CXXFLAGS) -pthread -c $< -o $@

server_Q7.o: server_Q7.cpp Graph.h Algorithms.h
	@echo "Compiling server_Q7.cpp → server_Q7.o"
	@$(CXX) $(CXXFLAGS) -pthread -c $< -o $@

server_Q8.o: server_Q8.cpp Graph.h Algorithms.h
	@echo "Compiling server_Q8.cpp → server_Q8.o"
	@$(CXX) $(CXXFLAGS) -pthread -c $< -o $@

server_Q9.o: server_Q9.cpp Graph.h Algorithms.h
	@echo "Compiling server_Q9.cpp → server_Q9.o"
	@$(CXX) $(CXXFLAGS) -pthread -c $< -o $@

# ------------------------------------------------------------------------------
# 9) Run fixed-input mode
# ------------------------------------------------------------------------------
run: $(TARGET)
	@echo "=== Running $(TARGET) with args: $(RUN_ARGS) ==="
	@./$(TARGET) $(RUN_ARGS)

# ------------------------------------------------------------------------------
# 10) Run random-graph mode
# ------------------------------------------------------------------------------
run_random: $(RANDOM_TARGET)
	@echo "=== Running $(RANDOM_TARGET) with args: $(RUN_RANDOM_ARGS) ==="
	@./$(RANDOM_TARGET) $(RUN_RANDOM_ARGS)

# ------------------------------------------------------------------------------
# 11) Run server Q6
# ------------------------------------------------------------------------------
run_server6: $(SERVER6_TARGET)
	@echo "=== Starting server Q6 on port 8080 ==="
	@./$(SERVER6_TARGET)

# ------------------------------------------------------------------------------
# 12) Run server Q7
# ------------------------------------------------------------------------------
run_server7: $(SERVER7_TARGET)
	@echo "=== Starting server Q7 on port 8080 ==="
	@./$(SERVER7_TARGET)

# ------------------------------------------------------------------------------
# 13) Run server Q8
# ------------------------------------------------------------------------------
run_server8: $(SERVER8_TARGET)
	@echo "=== Starting server Q8 on port 8080 ==="
	@./$(SERVER8_TARGET)

# ------------------------------------------------------------------------------
# 14) Run server Q9
# ------------------------------------------------------------------------------
run_server9: $(SERVER9_TARGET)
	@echo "=== Starting server Q9 on port 8080 ==="
	@./$(SERVER9_TARGET)

# ------------------------------------------------------------------------------
# 15) Code coverage build & instructions
# ------------------------------------------------------------------------------
coverage: CXXFLAGS += --coverage
coverage: clean $(TARGET)_cov

$(TARGET)_cov: Graph.o Algorithms.o main.o
	@echo "=== Building coverage instrumented binary ($(TARGET)_cov) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ">>> Use './$(
