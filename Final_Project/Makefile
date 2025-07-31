# Makefile for Final_Project — Stages 1–5 (Graph, Random, Profiling, Servers)

# Compiler and flags
CXX             := g++
CXXFLAGS        := -std=c++17 -Wall -Wextra -I.

# Targets
TARGET           := graph_app
RANDOM_TARGET    := random_app
SERVER6_TARGET   := server_Q6
SERVER7_TARGET   := server_Q7

# Sources
SRCS             := Graph.cpp Algorithms.cpp main.cpp
RANDOM_SRC       := random_graph.cpp
SERVER6_SRC      := server_Q6.cpp
SERVER7_SRC      := server_Q7.cpp

# Objects
OBJS             := $(SRCS:.cpp=.o)
RANDOM_OBJS      := Graph.o Algorithms.o random_graph.o
SERVER6_OBJS     := Graph.o Algorithms.o server_Q6.o
SERVER7_OBJS     := Graph.o Algorithms.o server_Q7.o

# Default arguments (override on command-line)
RUN_ARGS         ?= 3 0 1 1 2 2 0
RUN_RANDOM_ARGS  ?= -v 5 -e 7 -s 12345

.PHONY: all run run_random run_server6 run_server7 coverage profile memcheck callgrind clean clear

# ------------------------------------------------------------------------------
# 1) Default: build all executables
# ------------------------------------------------------------------------------
all: $(TARGET) $(RANDOM_TARGET) $(SERVER6_TARGET) $(SERVER7_TARGET)

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
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ">>> Built $(SERVER6_TARGET) <<<"

# ------------------------------------------------------------------------------
# 5) Build server Q7 executable
# ------------------------------------------------------------------------------
$(SERVER7_TARGET): $(SERVER7_OBJS)
	@echo "=== Linking $(SERVER7_TARGET) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ">>> Built $(SERVER7_TARGET) <<<"

# ------------------------------------------------------------------------------
# 6) Compile rules
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
	@$(CXX) $(CXXFLAGS) -c $< -o $@

server_Q7.o: server_Q7.cpp Graph.h Algorithms.h
	@echo "Compiling server_Q7.cpp → server_Q7.o"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# ------------------------------------------------------------------------------
# 7) Run fixed-input mode
# ------------------------------------------------------------------------------
run: $(TARGET)
	@echo "=== Running $(TARGET) with args: $(RUN_ARGS) ==="
	@./$(TARGET) $(RUN_ARGS)

# ------------------------------------------------------------------------------
# 8) Run random-graph mode
# ------------------------------------------------------------------------------
run_random: $(RANDOM_TARGET)
	@echo "=== Running $(RANDOM_TARGET) with args: $(RUN_RANDOM_ARGS) ==="
	@./$(RANDOM_TARGET) $(RUN_RANDOM_ARGS)

# ------------------------------------------------------------------------------
# 9) Run server Q6
# ------------------------------------------------------------------------------
run_server6: $(SERVER6_TARGET)
	@echo "=== Starting server Q6 on port 8080 ==="
	@./$(SERVER6_TARGET)

# ------------------------------------------------------------------------------
# 10) Run server Q7
# ------------------------------------------------------------------------------
run_server7: $(SERVER7_TARGET)
	@echo "=== Starting server Q7 on port 8080 ==="
	@./$(SERVER7_TARGET)

# ------------------------------------------------------------------------------
# 11) Code coverage build & instructions
# ------------------------------------------------------------------------------
coverage: CXXFLAGS += --coverage
coverage: clean $(TARGET)_cov

$(TARGET)_cov: Graph.o Algorithms.o main.o
	@echo "=== Building coverage instrumented binary ($(TARGET)_cov) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ">>> Use './$(TARGET)_cov $(RUN_ARGS)' then run 'gcov *.cpp' <<<"

# ------------------------------------------------------------------------------
# 12) gprof profiling build & instructions
# ------------------------------------------------------------------------------
profile: clean $(TARGET)_prof

$(TARGET)_prof: CXXFLAGS += -pg
$(TARGET)_prof: $(SRCS)
	@echo "=== Building gprof-instrumented binary (graph_app_prof) ==="
	@$(CXX) $(CXXFLAGS) -o graph_app_prof $^
	@echo ">>> Use './graph_app_prof $(RUN_ARGS)' then 'gprof graph_app_prof gmon.out > gprof_report.txt' <<<"

# ------------------------------------------------------------------------------
# 13) Valgrind Memcheck
# ------------------------------------------------------------------------------
memcheck: $(TARGET)
	@echo "=== Running Valgrind Memcheck on $(TARGET) ==="
	@valgrind --leak-check=full --show-leak-kinds=all \
        --log-file=memcheck_report.txt \
        ./$(TARGET) $(RUN_ARGS)
	@echo ">>> Memcheck report: memcheck_report.txt <<<"

# ------------------------------------------------------------------------------
# 14) Valgrind Callgrind profiling
# ------------------------------------------------------------------------------
callgrind: clean $(TARGET)
	@echo "=== Running Valgrind Callgrind on $(TARGET) ==="
	@valgrind --tool=callgrind \
	        --callgrind-out-file=callgrind.out \
	        ./$(TARGET) $(RUN_ARGS)
	@echo "=== Annotating Callgrind output ==="
	@callgrind_annotate callgrind.out > callgrind_report.txt
	@echo ">>> Callgrind report: callgrind_report.txt <<<"

# ------------------------------------------------------------------------------
# 15) Clean up all build artifacts
# ------------------------------------------------------------------------------
clean:
	@echo "=== Cleaning all build and analysis artifacts ==="
	@rm -f $(OBJS) $(RANDOM_OBJS) $(SERVER6_OBJS) $(SERVER7_OBJS) \
          $(TARGET) $(RANDOM_TARGET) $(SERVER6_TARGET) $(SERVER7_TARGET) \
          $(TARGET)_cov graph_app_prof gmon.out callgrind.out \
          memcheck_report.txt callgrind_report.txt *.gcda *.gcno *.gcov
	@echo ">>> All generated files have been removed <<<"

# ------------------------------------------------------------------------------
# 16) Alias for full clean
# ------------------------------------------------------------------------------
clear: clean
