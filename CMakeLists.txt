cmake_minimum_required(VERSION 3.23)
project(cheese)

set(CMAKE_CXX_STANDARD 23)
# fetch latest argparse
include(FetchContent)
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)
add_executable(cheese main.cpp lexer/lexer.h Coordinate.cpp Coordinate.h lexer/lexer.cpp NotImplementedException.h error.cpp error.h vterm.h vterm.cpp configuration.cpp configuration.h)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()
target_link_libraries(cheese argparse)