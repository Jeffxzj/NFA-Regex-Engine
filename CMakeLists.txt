cmake_minimum_required(VERSION 3.10)
project(regex)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -g3 -fno-omit-frame-pointer -fprofile-arcs -ftest-coverage -D __DEBUG__")

add_executable(regex
        src/main.cpp
        src/regex.cpp
        src/tokenizer.cpp
        src/parser.cpp
        src/reg_graph.cpp
        src/automata.cpp
)
