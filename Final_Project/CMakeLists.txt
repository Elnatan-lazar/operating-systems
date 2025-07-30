cmake_minimum_required(VERSION 3.15)
project(Final_Project)

# use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# tell CMake which sources to compile into the executable "graph_app"
add_executable(graph_app
        main.cpp
        Graph.cpp
        Algorithms.cpp
)
