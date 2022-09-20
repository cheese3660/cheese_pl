//
// Created by Lexi Allen on 9/18/2022.
//

#ifndef CHEESE_ERROR_H
#define CHEESE_ERROR_H
#include <cstdint>
#include <stdexcept>
#include <string>
#include "Coordinate.h"
namespace cheese::error {
    //Have an enum here in case
    constexpr uint32_t warning_modifier = 500;
    constexpr uint32_t lexer_error_start = 1000;
    constexpr uint32_t parser_error_start = 2000;
    constexpr uint32_t lowering_error_start = 3000;
    constexpr uint32_t generator_error_start = 4000;
    enum class ErrorCode : std::uint32_t {
        //Lexer errors first
        UnterminatedStringLiteral = lexer_error_start,
        UnterminatedCharacterLiteral,
        UnterminatedMultilineComment,

        //Then lexer warnings
        TodoFoundInComment = lexer_error_start+warning_modifier,
        BugFoundInComment,
        HackFoundInComment,
        FixmeFoundInComment,
        XXXFoundInComment,

        //Parsing errors next
        ExpectedOperator = 1000,


        //General error (thrown for example when multiple previous errors were printed)
        GeneralCompilerError = 9999,
    };

    char getErrorPrefix(std::uint32_t code);
    std::string getError(std::uint32_t code);
    std::string getError(ErrorCode code);

    class CompilerError : public std::runtime_error {
    public:
        Coordinate location;
        ErrorCode code;

        // Construct with given error message:
        CompilerError(Coordinate location, ErrorCode code, const char *message)
                : std::runtime_error(message), location(location), code(code) {
        }
        const char* what();
    };


}

#endif //CHEESE_ERROR_H
