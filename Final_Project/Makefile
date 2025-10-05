# ==============================================================================
# Compiler and flags
# ==============================================================================
CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -I. -pthread

# ==============================================================================
# Targets (executables)
# ==============================================================================
TARGET       := graph            # Q1-2: Graph + Euler
RANDOM_TARGET:= random           # Q3: Random graph
SERVER6      := server_Q6        # Q6: Server - Euler
CLIENT6      := client_Q6
SERVER7      := server_Q7        # Q7: Server - 4 algorithms + Factory/Strategy
CLIENT7	     := client_Q7
SERVER8      := server_Q8        # Q8: Server - Multithreading (Load Factor)
CLIENT8      := client_Q8
SERVER9      := server_Q9        # Q9: Server - Multithreading (Pipeline)
CLIENT9      := client_Q9

# ==============================================================================
# Sources
# ==============================================================================
SRCS        := Graph.cpp Algorithms.cpp main.cpp
RANDOM_SRC  := Graph.cpp Algorithms.cpp random_graph.cpp
SERVER6_SRC := Graph.cpp Algorithms.cpp server_Q6.cpp
SERVER7_SRC := Graph.cpp Algorithms.cpp server_Q7.cpp
SERVER8_SRC := Graph.cpp Algorithms.cpp server_Q8.cpp
SERVER9_SRC := Graph.cpp Algorithms.cpp server_Q9.cpp

# ==============================================================================
# Objects
# ==============================================================================
OBJS        := $(SRCS:.cpp=.o)
RANDOM_OBJS := $(RANDOM_SRC:.cpp=.o)
SERVER6_OBJS:= $(SERVER6_SRC:.cpp=.o)
SERVER7_OBJS:= $(SERVER7_SRC:.cpp=.o)
SERVER8_OBJS:= $(SERVER8_SRC:.cpp=.o)
SERVER9_OBJS:= $(SERVER9_SRC:.cpp=.o)

# ==============================================================================
# Default arguments
# ==============================================================================
RUN_ARGS        ?= 3 0 1 1 2 2 0
RUN_RANDOM_ARGS ?= -v 5 -e 10 -s 12345

.PHONY: all clean clear \
        run run_random \
        run_server6 run_server7 run_server8 run_server9 \
        coverage_basic coverage_full report_basic report_full \
        profile memcheck callgrind helgrind_all memcheck_all callgrind_all

# ==============================================================================
# Build all object files first (optional)
# ==============================================================================
all: $(OBJS) $(RANDOM_OBJS) $(SERVER6_OBJS) $(SERVER7_OBJS) $(SERVER8_OBJS) $(SERVER9_OBJS)
	@echo "=== All object files built ==="

# ==============================================================================
# Q1-2: Graph + Euler
# ==============================================================================
# make run
$(TARGET): $(OBJS)
	@echo "=== Linking $(TARGET) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^

run: $(TARGET)
	@echo "=== Running $(TARGET) with args: $(RUN_ARGS) ==="
	@./$(TARGET) $(RUN_ARGS)

# ==============================================================================
# Q3: Random graph
# ==============================================================================
# make run_random
$(RANDOM_TARGET): $(RANDOM_OBJS)
	@echo "=== Linking $(RANDOM_TARGET) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^

run_random: $(RANDOM_TARGET)
	@echo "=== Running $(RANDOM_TARGET) with args: $(RUN_RANDOM_ARGS) ==="
	@./$(RANDOM_TARGET) $(RUN_RANDOM_ARGS)

# ==============================================================================
# Q4: Coverage + Profiling + Valgrind (basic code)
# ==============================================================================
# make coverage_basic
# ./graph_cov 3 0 1 1 2 2 0
# make report_basic
# (make open_report_basic)
# make profile
# make memcheck
# make callgrind
coverage_basic: CXXFLAGS += --coverage
coverage_basic: clean graph_cov

graph_cov: $(OBJS)
	@echo "=== Building coverage binary ($(TARGET)_cov) ==="
	@$(CXX) $(CXXFLAGS) -o $@ $^

report_basic:
	@echo "=== Coverage Report ==="
	@for f in Graph.cpp Algorithms.cpp main.cpp; do \
		gcov -b -c $$f | grep -A5 "File '$$f'"; \
		echo ""; \
	done

open_report_basic:
	@echo "=== Opening coverage files ==="
	@less Graph.cpp.gcov
	@less Algorithms.cpp.gcov
	@less main.cpp.gcov

profile: CXXFLAGS += -pg
profile: clean $(TARGET)
	@echo "=== Building profiling binary ($(TARGET)) ==="
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "=== Running profiling $(TARGET) ==="
	@./$(TARGET) $(RUN_ARGS)
	@gprof $(TARGET) gmon.out > profile_report.txt

open_profile: profile
	@echo "=== Opening gprof profile report ==="
	@gprof $(TARGET) gmon.out > profile_report.txt
	@less profile_report.txt

memcheck: clean $(TARGET)
	@echo "=== Running Valgrind memcheck ==="
	valgrind --leak-check=full ./$(TARGET) $(RUN_ARGS)

callgrind: clean $(TARGET)
	@echo "=== Running Valgrind callgrind ==="
	valgrind --tool=callgrind ./$(TARGET) $(RUN_ARGS)

# ==============================================================================
# Q6-Q9: Servers
# ==============================================================================
# make clean
# make run_server6 .. run_server9
# new wsl: ./client_Q6 ..  ./client_Q9
$(SERVER6): $(SERVER6_OBJS)
	@echo "=== Linking $(SERVER6) ==="
	@$(CXX) $(CXXFLAGS) -pthread -o $@ $^

$(CLIENT6): client_Q6.cpp
	@echo "=== Building $(CLIENT6) ==="
	@$(CXX) $(CXXFLAGS) -o $(CLIENT6) client_Q6.cpp

run_server6: $(SERVER6) $(CLIENT6)
	@echo "=== Starting $(SERVER6) on port 8080 ==="
	@./$(SERVER6)

$(SERVER7): $(SERVER7_OBJS)
	@echo "=== Linking $(SERVER7) ==="
	@$(CXX) $(CXXFLAGS) -pthread -o $@ $^

$(CLIENT7): client_Q7.cpp
	@echo "=== Building $(CLIENT7) ==="
	@$(CXX) $(CXXFLAGS) -o $(CLIENT7) client_Q7.cpp

run_server7: $(SERVER7) $(CLIENT7)
	@echo "=== Starting $(SERVER7) on port 8080 ==="
	@./$(SERVER7)

$(SERVER8): $(SERVER8_OBJS)
	@echo "=== Linking $(SERVER8) ==="
	@$(CXX) $(CXXFLAGS) -pthread -o $@ $^

$(CLIENT8): client_Q8.cpp
	@echo "=== Building $(CLIENT8) ==="
	@$(CXX) $(CXXFLAGS) -o $(CLIENT8) client_Q8.cpp

run_server8: $(SERVER8) $(CLIENT8)
	@echo "=== Starting $(SERVER8) on port 8080 ==="
	@./$(SERVER8)

$(SERVER9): $(SERVER9_OBJS)
	@echo "=== Linking $(SERVER9) ==="
	@$(CXX) $(CXXFLAGS) -pthread -o $@ $^

$(CLIENT9): client_Q9.cpp
	@echo "=== Building $(CLIENT9) ==="
	@$(CXX) $(CXXFLAGS) -o $(CLIENT9) client_Q9.cpp

run_server9: $(SERVER9) $(CLIENT9)
	@echo "=== Starting $(SERVER9) on port 8080 ==="
	@./$(SERVER9)

# ==============================================================================
# Q10: Advanced Valgrind for servers
# ==============================================================================
# make memcheck_all
# make callgrind_all
# make helgrind_all
memcheck_all: $(SERVER6) $(SERVER7) $(SERVER8) $(SERVER9)
	@echo "=== Valgrind memcheck for all servers ==="
	valgrind --leak-check=full ./$(SERVER6)
	valgrind --leak-check=full ./$(SERVER7)
	valgrind --leak-check=full ./$(SERVER8)
	valgrind --leak-check=full ./$(SERVER9)

callgrind_all: $(SERVER6) $(SERVER7) $(SERVER8) $(SERVER9)
	@echo "=== Valgrind callgrind for all servers ==="
	valgrind --tool=callgrind ./$(SERVER6)
	valgrind --tool=callgrind ./$(SERVER7)
	valgrind --tool=callgrind ./$(SERVER8)
	valgrind --tool=callgrind ./$(SERVER9)

helgrind_all: $(SERVER6) $(SERVER7) $(SERVER8) $(SERVER9)
	@echo "=== Valgrind helgrind for all servers ==="
	valgrind --tool=helgrind ./$(SERVER6)
	valgrind --tool=helgrind ./$(SERVER7)
	valgrind --tool=helgrind ./$(SERVER8)
	valgrind --tool=helgrind ./$(SERVER9)

# ==============================================================================
# Q11: Full coverage (servers)
# ==============================================================================
# make coverage_full
# make report_full
COV_FLAGS = --coverage

coverage_full: CXXFLAGS += $(COV_FLAGS)
coverage_full: clean server6_cov server7_cov server8_cov server9_cov

server6_cov: $(SERVER6) $(CLIENT6)
	@echo "=== [COVERAGE] Running Q6 server & client ==="
	(./$(SERVER6) & pid=$$!; sleep 1; \
	 echo "CREATE undirected 5 3 (0,1) (1,2) (2,0)\nQ" | ./$(CLIENT6); \
	 wait $$pid)

server7_cov: $(SERVER7) $(CLIENT7)
	@echo "=== [COVERAGE] Running Q7 server & client ==="
	(./$(SERVER7) & pid=$$!; sleep 1; \
	 echo "RANDOM directed 4 6 23232\nQ" | ./$(CLIENT7); \
	 wait $$pid)

server8_cov: $(SERVER8) $(CLIENT8)
	@echo "=== [COVERAGE] Running Q8 server & client ==="
	(./$(SERVER8) & pid=$$!; sleep 1; \
	 echo "CREATE directed 6 5 (0,1) (1,2) (2,3) (3,4) (4,5)\nQ" | ./$(CLIENT8); \
	 wait $$pid)

server9_cov: $(SERVER9) $(CLIENT9)
	@echo "=== [COVERAGE] Running Q9 server & client ==="
	(./$(SERVER9) & pid=$$!; sleep 1; \
	 echo "RANDOM undirected 5 8 12345\nQ" | ./$(CLIENT9); \
	 wait $$pid)

report_full:
	@echo "=== COVERAGE REPORT ==="
	@total=0; count=0; \
	for f in Graph.cpp Algorithms.cpp server_Q6.cpp server_Q7.cpp server_Q8.cpp server_Q9.cpp; do \
	    echo ">>> $$f <<<"; \
	    percent=$$(gcov -n $$f | grep "Lines executed" | head -n 1 | sed 's/.*: *\([0-9.]*%\).*/\1/'); \
	    if [ -z "$$percent" ]; then \
	        echo "No coverage info"; \
	    else \
	        echo "Coverage: $$percent"; \
	    fi; \
	    echo; \
	done

# ==============================================================================
# Clean and clear
# ==============================================================================
clean:
	@rm -f *.o \
	       $(TARGET) $(RANDOM_TARGET) $(SERVER6) $(CLIENT6) \
	       $(SERVER7) $(CLIENT7) \
	       $(SERVER8) $(CLIENT8) \
	       $(SERVER9) $(CLIENT9) \
	       $(TARGET)_cov graph_cov graph_fullcov \
	       *.gcno *.gcda *.gcov gmon.out profile_report.txt \
		   callgrind.out.*

clear: clean
	@rm -f *.txt
