//
// Created by Lexi Allen on 9/18/2022.
//

#include "error.h"
#include <string>
#include <cstdint>
#include <sstream>
namespace cheese::error {

    const char *CompilerError::what() {
        auto file_plus_codename = (location.toString() + ": ") + getError(code);
        auto complete = file_plus_codename + " " + std::runtime_error::what();
        return complete.c_str();
    }

    char getErrorPrefix(std::uint32_t code) {
        if (code % 1000 >= 500) {
            return 'W';
        }
        if (code >= lexer_error_start && code < parser_error_start) {
            return 'L';
        } else if (code >= parser_error_start && code < lowering_error_start) {
            return 'P';
        } else if (code >= lowering_error_start && code < generator_error_start) {
            return 'C';
        } else if (code >= generator_error_start) {
            return 'G';
        }


        return 'U';
    }
    std::string getError(std::uint32_t code) {
        return getErrorPrefix(code) + std::to_string(code);
    }
    std::string getError(ErrorCode code) {
        return getError(static_cast<std::uint32_t>(code));
    }
}