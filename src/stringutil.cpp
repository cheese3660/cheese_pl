//
// Created by Lexi Allen on 4/23/2023.
//
#include "stringutil.h"
#include <sstream>
#include <iomanip>

namespace cheese::stringutil {
    std::string escape(std::string const &str) {
        std::ostringstream result;
        for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
            if (' ' <= *it && *it <= '~') {
                result << *it;
            } else {
                switch (*it) {
                    case '\a':
                        result << "\\a";
                        break;
                    case '\b':
                        result << "\\b";
                        break;
                    case '\f':
                        result << "\\f";
                        break;
                    case '\n':
                        result << "\\n";
                        break;
                    case '\r':
                        result << "\\r";
                        break;
                    case '\t':
                        result << "\\t";
                        break;
                    case '\v':
                        result << "\\v";
                        break;
                    case '\\':
                        result << "\\\\";
                    default:
                        result << "\\x" << std::setw(2) << std::hex << std::setfill('0') << *it;
                }
            }
            return result.str();
        }
    }
}