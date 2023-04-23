//
// Created by Lexi Allen on 4/23/2023.
//
#include "curdle/names.h"
#include <map>
#include <sstream>

namespace cheese::curdle {
    static std::map<char, char> translations{};
    static std::map<char, char> untranslations{};
    static bool defined{false};

    static void define() {
        if (defined) return;
        translations['.'] = 'D';
        translations['('] = 'L';
        translations[')'] = 'R';
        translations[','] = 'C';
        translations['_'] = '_';
        translations['['] = 'o';
        translations[']'] = 'c';
        translations['&'] = 'A';
        translations['*'] = 'P';
        translations['~'] = 'T';
        translations[' '] = 'S';
        translations['"'] = 'Q';
        translations['!'] = 'e';
        translations['#'] = 'h';
        translations['$'] = 'd';
        translations['%'] = 'p';
        translations['\''] = 'q';
        translations['+'] = 'a';
        translations['-'] = 'm';
        translations['/'] = 's';
        translations[':'] = '0';
        translations[';'] = '1';
        translations['<'] = '2';
        translations['='] = 'E';
        translations['>'] = 'g';
        translations['\\'] = 'b';
        translations['^'] = '3';
        translations['`'] = '4';
        translations['{'] = '5';
        translations['|'] = '6';
        translations['}'] = '7';
        translations['?'] = '8';
        for (auto translation: translations) {
            untranslations[translation.second] = translation.first;
        }
        defined = true;
    }

    static std::string translate(char original) {
        if (translations.contains(original)) {
            return std::string{"_"} + translations[original];
        } else {
            return std::string{""} + translations[original];
        }
    }

    std::string
    mangle(std::string path, std::vector<std::string> typenames, std::string returnTypeName, bool isExtern) {
        define();
        if (isExtern) {
            return path;
        } else {
            std::stringstream ss{};
            for (char c: path) {
                ss << translate(c);
            }
            for (auto &tn: typenames) {
                ss << "_a";
                for (char c: tn) {
                    ss << translate(c);
                }
            }
            ss << "_r";
            for (char c: returnTypeName) {
                ss << translate(c);
            }
            return ss.str();
        }
    }

    std::string mangle(std::string variable_name, bool isExtern) {
        define();
        if (isExtern) {
            return variable_name;
        }
        std::stringstream ss{};
        for (char c: variable_name) {
            ss << translate(c);
        }
        return ss.str();
    }

    std::string unmangle_variable(std::string mangled) {
        define();
        std::stringstream ss{};
        for (int i = 0; i < mangled.length(); i++) {
            char c = mangled[i];
            if (c == '_') {
                ss << untranslations[mangled[i + 1]];
                i++;
            } else {
                ss << c;
            }
        }
        return ss.str();
    }

    std::string unmangle_function(std::string mangled) {
        define();
        std::stringstream ss{};
        bool first_arg = true;
        for (int i = 0; i < mangled.length(); i++) {
            char c = mangled[i];
            if (c == '_') {
                char c2 = mangled[i + 1];
                switch (c2) {
                    case 'a':
                        if (first_arg) {
                            ss << ' ';
                            first_arg = false;
                        } else {
                            ss << ", ";
                        }
                        break;
                    case 'r':
                        ss << " => ";
                        break;
                    default:
                        ss << untranslations[c2];
                        break;
                }
                i++;
            } else {
                ss << c;
            }
        }
        return ss.str();
    }

    std::string combine_names(const std::string &a, std::string b) {
        if (a.empty()) {
            return b;
        } else {
            return a + '.' + b;
        }
    }
}