cmake_minimum_required(VERSION 3.23)
project(cheese)

set(CMAKE_CXX_STANDARD 23)
# fetch latest argparse
include(FetchContent)
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)

FetchContent_MakeAvailable()
set(JSON_Install ON)
FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        GIT_TAG v3.11.2)
FetchContent_MakeAvailable(json argparse)
include_directories(include)
# holy shit llvm was fun to build for windows
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(cheese src/main.cpp include/lexer/lexer.h src/Coordinate.cpp include/Coordinate.h src/lexer/lexer.cpp include/NotImplementedException.h src/error.cpp include/error.h include/vterm.h src/vterm.cpp src/configuration.cpp include/configuration.h include/tools/tools.h src/tools/tools.cpp src/tools/test.cpp include/tools/test.h include/tests/tests.h src/tests/tests.cpp src/tests/lexer_tests.cpp)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(cheese argparse ${llvm_libs} nlohmann_json::nlohmann_json)
