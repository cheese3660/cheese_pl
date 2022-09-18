cmake_minimum_required(VERSION 3.23)
project(cheese)

set(CMAKE_CXX_STANDARD 23)

add_executable(cheese main.cpp lexer/lexer.h Coordinate.cpp Coordinate.h lexer/lexer.cpp NotImplementedException.h)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()
