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
FetchContent_MakeAvailable(argparse)
include_directories(include)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
if (CMAKE_BUILD_TYPE MATCHES Debug)
    message("debug mode")
    add_definitions(-DDEBUG)
endif ()
# Uncomment the following line out to remove self-tests from the compiler, self-tests add to the final size, and also add to the startup time marginally
# add_compile_definitions(CHEESE_NO_SELF_TESTS)


add_executable(
        cheese
        src/main.cpp
        include/lexer/lexer.h
        src/Coordinate.cpp
        include/Coordinate.h
        src/lexer/lexer.cpp
        include/NotImplementedException.h
        src/error.cpp include/error.h
        include/vterm.h
        src/vterm.cpp
        src/configuration.cpp
        include/configuration.h
        include/tools/tools.h
        src/tools/tools.cpp
        src/tools/test.cpp
        include/tools/test.h
        include/tests/tests.h
        src/tests/tests.cpp
        src/tests/lexer_tests.cpp
        src/parser/Node.cpp
        include/parser/Node.h
        include/parser/parser.h
        src/parser/nodes/Structure.cpp
        src/parser/parser.cpp
        src/tests/parser_tests.cpp
        include/compression/base64.h
        src/compression/base64.cpp
        external/inflatecpp/adler32.cc
        external/inflatecpp/bit_reader.cc
        external/inflatecpp/crc32.cc
        external/inflatecpp/decompressor.cc
        external/inflatecpp/huffman_decoder.cc
        src/parser/nodes/Import.cpp
        include/parser/nodes/terminal_nodes.h
        include/parser/nodes/single_member_nodes.h
        include/math/BigInteger.h
        src/math/BigInteger.cpp
        include/parser/nodes/other_nodes.h
        include/tools/parse.h
        src/tools/parse.cpp
        include/curdle/curdle.h
        src/bacteria/BacteriaNode.cpp
        include/bacteria/BacteriaNode.h
        include/bacteria/BacteriaType.h
        src/curdle/Project.cpp
        include/curdle/Project.h
        src/curdle/curdle.cpp
        include/curdle/Structure.h
        include/curdle/FunctionTemplate.h
        include/curdle/Type.h
        include/curdle/Mixin.h
        include/curdle/GlobalContext.h
        include/curdle/variables.h
        include/curdle/comptime.h
        include/memory/garbage_collection.h
        src/memory/garbage_collection.cpp
        src/curdle/Structure.cpp
        src/curdle/comptime.cpp
        src/curdle/Type.cpp
        src/curdle/Mixin.cpp
        include/curdle/Interface.h
        src/curdle/Interface.cpp
        include/curdle/functions.h
        src/curdle/functions.cpp
        src/tools/translate.cpp
        include/tools/translate.h
        src/curdle/GlobalContext.cpp
        include/bacteria/BacteriaReciever.h
        include/bacteria/nodes/reciever_nodes.h
        include/curdle/runtime.h
        src/curdle/runtime.cpp src/curdle/runtime.cpp include/curdle/names.h src/curdle/names.cpp include/stringutil.h src/stringutil.cpp src/bacteria/BacteriaType.cpp include/bacteria/nodes/expression_nodes.h include/curdle/builtin.h src/curdle/builtin.cpp src/tests/math_tests.cpp)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif ()
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(cheese argparse ${llvm_libs})
