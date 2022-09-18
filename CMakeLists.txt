cmake_minimum_required(VERSION 3.23)
project(cheese)

set(CMAKE_CXX_STANDARD 23)

add_executable(cheese main.cpp lexer/lexer.h Coordinate.cpp Coordinate.h lexer/lexer.cpp)
