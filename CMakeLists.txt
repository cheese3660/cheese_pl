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
        src/project/Project.cpp
        include/project/Project.h
        src/curdle/curdle.cpp
        include/curdle/types/Structure.h
        include/curdle/FunctionTemplate.h
        include/curdle/Type.h
        include/curdle/Mixin.h
        include/project/GlobalContext.h
        include/curdle/variables.h
        include/curdle/comptime.h
        include/memory/garbage_collection.h
        src/memory/garbage_collection.cpp
        src/curdle/types/Structure.cpp
        src/curdle/comptime.cpp
        src/curdle/Type.cpp
        src/curdle/Mixin.cpp
        include/curdle/Interface.h
        src/curdle/Interface.cpp
        include/curdle/functions.h
        src/curdle/functions.cpp
        src/tools/translate.cpp
        include/tools/translate.h
        src/project/GlobalContext.cpp
        include/bacteria/BacteriaReceiver.h
        include/bacteria/nodes/reciever_nodes.h
        include/curdle/runtime.h
        src/curdle/runtime.cpp
        src/curdle/runtime.cpp
        include/curdle/names.h
        src/curdle/names.cpp
        include/stringutil.h
        src/stringutil.cpp
        src/bacteria/BacteriaType.cpp
        include/bacteria/nodes/expression_nodes.h
        include/curdle/builtin.h
        src/curdle/builtin.cpp
        src/tests/math_tests.cpp
        src/bacteria/nodes/reciever_nodes.cpp
        src/tests/curdle_tests.cpp
        src/tests/curdle_tests.cpp
        include/curdle/values/ComptimeInteger.h
        src/curdle/values/ComptimeInteger.cpp
        include/util/json_template.h
        src/util/json_template.cpp
        include/curdle/values/ComptimeFloat.h
        src/curdle/values/ComptimeFloat.cpp
        include/curdle/values/ComptimeString.h
        src/curdle/values/ComptimeString.cpp
        include/curdle/values/ComptimeVoid.h
        src/curdle/values/ComptimeVoid.cpp
        include/curdle/values/ComptimeComplex.h
        src/curdle/values/ComptimeComplex.cpp
        include/curdle/values/ComptimeType.h
        src/curdle/values/ComptimeType.cpp
        include/curdle/values/ComptimeArray.h
        src/curdle/values/ComptimeArray.cpp
        include/curdle/values/ComptimeObject.h
        src/curdle/values/ComptimeObject.cpp
        include/curdle/values/ComptimeFunctionSet.h
        src/curdle/values/ComptimeFunctionSet.cpp
        include/curdle/values/BuiltinFunctionReference.h
        src/curdle/values/BuiltinFunctionReference.cpp
        include/curdle/types/TypeType.h
        src/curdle/types/TypeType.cpp
        include/curdle/types/ComptimeIntegerType.h
        src/curdle/types/ComptimeIntegerType.cpp
        include/curdle/types/IntegerType.h
        src/curdle/types/IntegerType.cpp
        include/curdle/types/ReferenceType.h
        src/curdle/types/ReferenceType.cpp
        include/curdle/types/VoidType.h
        src/curdle/types/VoidType.cpp
        include/curdle/types/NoReturnType.h
        src/curdle/types/NoReturnType.cpp
        include/curdle/types/AnyType.h
        src/curdle/types/AnyType.cpp
        include/curdle/types/BooleanType.h
        src/curdle/types/BooleanType.cpp
        include/curdle/types/Float64Type.h
        src/curdle/types/Float64Type.cpp
        include/curdle/types/ComptimeFloatType.h
        src/curdle/types/ComptimeFloatType.cpp
        include/curdle/types/FunctionTemplateType.h
        src/curdle/types/FunctionTemplateType.cpp
        include/curdle/types/BuiltinReferenceType.h
        src/curdle/types/BuiltinReferenceType.cpp
        include/curdle/types/ErrorType.h
        src/curdle/types/ErrorType.cpp
        include/curdle/types/Complex64Type.h
        src/curdle/types/Complex64Type.cpp
        include/curdle/types/ComptimeComplexType.h
        src/curdle/types/ComptimeComplexType.cpp
        include/project/Machine.h
        include/util/llvm_utils.h
        src/util/llvm_utils.cpp
        include/util/Endianness.h
        include/curdle/values/ComptimeBool.h
        src/curdle/values/ComptimeBool.cpp
        src/bacteria/BacteriaReceiver.cpp
        include/curdle/types/ComptimeStringType.h
        src/curdle/types/ComptimeStringType.cpp
        include/curdle/types/FunctionPointerType.h
        src/curdle/types/FunctionPointerType.cpp
        include/curdle/types/ComptimeEnumType.h
        src/curdle/types/ComptimeEnumType.cpp
        include/curdle/values/ComptimeEnumLiteral.h
        src/curdle/values/ComptimeEnumLiteral.cpp
        include/curdle/types/ComposedFunctionType.h
        include/curdle/types/ComptimeComposedFunctionType.h
        include/curdle/enums/SimpleOperation.h
        src/curdle/types/ComposedFunctionType.cpp
        src/curdle/enums/SimpleOperation.cpp include/curdle/types/ArrayType.h include/curdle/types/PointerType.h src/curdle/types/ArrayType.cpp src/curdle/types/PointerType.cpp include/curdle/types/ImportedFunctionType.h src/curdle/types/ImportedFunctionType.cpp include/curdle/values/ImportedFunction.h src/curdle/values/ImportedFunction.cpp include/bacteria/BacteriaContext.h include/bacteria/FunctionContext.h include/bacteria/ScopeContext.h include/bacteria/WriteContext.h src/bacteria/BacteriaContext.cpp include/tools/lower.h src/tools/lower.cpp src/bacteria/nodes/expression_nodes.cpp include/bacteria/FunctionInfo.h include/bacteria/VariableInfo.h src/bacteria/FunctionContext.cpp src/bacteria/ScopeContext.cpp src/bacteria/VariableInfo.cpp include/bacteria/ExpressionContext.h src/tools/build.cpp include/tools/build.h)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif ()
llvm_map_components_to_libnames(llvm_libs ${LLVM_TARGETS_TO_BUILD} support core irreader codegen mc mcparser option)
target_link_libraries(cheese argparse ${llvm_libs})
