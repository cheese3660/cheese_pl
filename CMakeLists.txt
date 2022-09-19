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
include_directories(include)

add_executable(cheese src/main.cpp include/lexer/lexer.h src/Coordinate.cpp include/Coordinate.h src/lexer/lexer.cpp include/NotImplementedException.h src/error.cpp include/error.h include/vterm.h src/vterm.cpp src/configuration.cpp include/configuration.h)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()
target_link_libraries(cheese argparse)