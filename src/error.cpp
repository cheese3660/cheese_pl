//
// Created by Lexi Allen on 9/18/2022.
//

#include "error.h"
#include "vterm.h"
#include <string>
#include <cstdint>
#include <sstream>
#include <iostream>
namespace cheese::error {

    std::string CompilerError::what() {
        auto file_plus_codename = (location.toString() + ": ") + getError(code);
        auto complete = file_plus_codename + " " + std::runtime_error::what();
        return complete;
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

    void write_module(const char* module) {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Blue);
        }
        std::cout << "[" << module << "] ";
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
    }

    void write_err() {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Red);
        }
        std::cout << "[ERROR] ";
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
    }

    void write_warn() {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Red);
        }
        std::cout << "[WARN] ";
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
    }

    void write_code(ErrorCode code) {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::DarkGray);
        }
        std::cout << "(" << getError(code) << ") ";
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
    }

    void write_location(Coordinate location) {
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::White);
        }
        std::cout << location.toString();
        if (configuration::use_escape_sequences) {
            io::setForegroundColor(io::Color::Gray);
        }
        std::cout << ": ";
    }

    [[noreturn]] void raise_exiting_error(const char* module, std::string message, Coordinate location, ErrorCode code) {
        if (configuration::log_errors) {
            write_module(module);
            write_err();
            write_code(code);
            write_location(location);
            std::cout << message << '\n';
        }
        throw CompilerError(location, code, message);
    }

    void raise_error(const char* module, std::string message, Coordinate location, ErrorCode code) {
        if (configuration::die_on_first_error) {
            raise_exiting_error(module,message,location,code);
        }
        if (configuration::log_errors) {
            write_module(module);
            write_err();
            write_code(code);
            write_location(location);
            std::cout << message << '\n';
        }
    }

    void raise_warning(const char* module, std::string message, Coordinate location, ErrorCode code) {
        if (configuration::warnings_are_errors) {
            raise_error(module, message, location, code);
        }
        if (configuration::log_errors) {
            write_module(module);
            write_warn();
            write_code(code);
            write_location(location);
            std::cout << message << '\n';
        }
    }
}