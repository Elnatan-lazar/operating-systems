# make run --> only timing check
# make profile --> show profile report of functions

CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = ch_profile
SRC = ch_profile.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

profile: clean
	$(CXX) $(CXXFLAGS) -pg -o $(TARGET) $(SRC)
	./$(TARGET)
	@echo "=== Generating profiling report ==="
	gprof $(TARGET) gmon.out > profile_report.txt
	grep -E "convexHull|cross|Point::operator<" profile_report.txt > profile_report_filtered.txt
	@echo "Profiling complete! Report saved to profile_report.txt"

clean:
	rm -f $(TARGET) timing_results.txt gmon.out profile_report.txt profile_report_filtered.txt
