//
// Created by Lexi Allen on 9/21/2022.
//
#ifndef CHEESE_NO_SELF_TESTS
#include "tests/tests.h"
#include "parser/parser.h"

namespace cheese::tests::parser_tests {
    TEST_SECTION("parser", 1)

        TEST_SUBSECTION("parser validation")
            struct parser_validation_test {
                const char* name;
                const char* source;
                const char* expected;
            };
            const parser_validation_test validation_tests[] {
                    {
                        "simple entry",
                        "fn main => u8 entry ==> 1",
                        R"({"type":"structure","tuple":false,"children":[{"type":"function_definition","modifiers":["entry"],"name":"main","arguments":[],"return_type":{"type":"unsigned_int_type","size":8},"body":{"type":"return","value":{"type":"integer","value":1}}}]})"
                    },
                    {
                        "function calls",
                        "fn main => u8 entry ==> function(3)",
                        R"({"type":"structure","tuple":false,"children":[{"type":"function_definition","modifiers":["entry"],"name":"main","arguments":[],"return_type":{"type":"unsigned_int_type","size":8},"body":{"type":"return","value":{"type":"function_call","function":{"type":"reference","value":"function"},"arguments":[{"type":"integer","value":3}]}}}]})"
                    },
                    {
                        "newlines separate expressions",
                        "fn abs arg: f64 => f64\n"
                        "    ==>\n"
                        "        if arg < 0"
                        "             -arg"
                        "        else"
                        "             arg",
                        R"({"type":"structure","tuple":false,"children":[{"type":"function_definition","modifiers":[],"name":"abs","arguments":[{"type":"function_argument","comptime":false,"name":"arg","arg_type":{"type":"f64_type"}}],"return_type":{"type":"f64_type"},"body":{"type":"return","value":{"type":"if","condition":{"type":"less_than","lhs":{"type":"reference","value":"arg"},"rhs":{"type":"integer","value":0}},"body":{"type":"unary_minus","child":{"type":"reference","value":"arg"}},"else":{"type":"reference","value":"arg"}}}}]})"
                    },
                    {
                        "then separates conditions",
                        "fn abs arg: f64 => f64 ==> if arg < 0 then -arg else arg",
                        R"({"type":"structure","tuple":false,"children":[{"type":"function_definition","modifiers":[],"name":"abs","arguments":[{"type":"function_argument","comptime":false,"name":"arg","arg_type":{"type":"f64_type"}}],"return_type":{"type":"f64_type"},"body":{"type":"return","value":{"type":"if","condition":{"type":"less_than","lhs":{"type":"reference","value":"arg"},"rhs":{"type":"integer","value":0}},"body":{"type":"unary_minus","child":{"type":"reference","value":"arg"}},"else":{"type":"reference","value":"arg"}}}}]})"
                    },
                    {
                        "if nesting",
                        "fn test => u8 ==> if 1 then 2 else if 3 then 4 else 5",
                        R"({"type":"structure","tuple":false,"children":[{"type":"function_definition","modifiers":[],"name":"test","return_type":{"type":"unsigned_int_type","size":8},"body":{"type":"return","value":{"type":"if","condition":{"type":"integer","value":1},"body":{"type":"integer","value":2},"else":{"type":"if","condition":{"type":"integer","value":3},"body":{"type":"integer","value":4},"else":{"type":"integer","value":5}}}}}]})"
                    },
                    {
                        "while loops",
                        "fn test => void while 1 do 2",
                        R"({"type":"structure","tuple":false,"children":[{"type":"function_definition","modifiers":[],"name":"test","return_type":{"type":"void"},"body":{"type":"return","value":{"type":"while","condition":{"type":"integer","value":1},"body":{"type":"integer","value":2}}}}]})"
                    },
                    {
                        "tuples",
                        "let tup = struct (u8 u8 u8)",
                        R"({"type":"structure","tuple":false,"children":[{"type":"variable_declaration","variable":{"type": "variable_definition", "name":"tup","modifiers":[]},"value":{"type":"structure","tuple":true,"children":[{"type":"unsigned_int_type","size":8},{"type":"unsigned_int_type","size":8},{"type":"unsigned_int_type","size":8}]}}]})"
                    },
                    {
                        "empty structure",
                        "let empty = struct",
                        R"({"type":"structure","tuple":false,"children":[{"type":"variable_declaration","variable":{"type": "variable_definition", "name":"empty","modifiers":[]},"value":{"type":"structure","tuple":false,"children":[]}}]})"
                    },
                    //TODO ADD A TEST FOR A FILLED STRUCTURE HERE
                    {
                        "array literal",
                        "let arr: []u8 = .[1 2 3]",
                        R"({"type":"structure","tuple":false,"children":[{"type":"variable_declaration","variable":{"type":"variable_definition","name":"arr","variable_type":{"type":"array_type","child":{"type":"unsigned_int_type","size":8}},"modifiers":[]},"value":{"type":"array_literal","children":[{"type":"integer","value":1},{"type":"integer","value":2},{"type":"integer","value":3}]}}]})"
                    }

                    //TODO ADD TESTS FOR NOT YET IMPLEMENTED FEATURES AS WELL TO VALIDATE THEM
            };

            TEST_GENERATOR {
            }
        TEST_END
        TEST_SUBSECTION("parser errors")
            TEST_GENERATOR {

            }
        TEST_END
    TEST_END
}
#endif