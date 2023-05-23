//
// Created by Lexi Allen on 4/23/2023.
//

#ifndef CHEESE_NAMES_H
#define CHEESE_NAMES_H

#include <string>
#include <vector>

namespace cheese::curdle {
    std::string
    mangle(std::string path, const std::vector<std::string> &typenames, std::string returnTypeName, bool isExtern);

    std::string mangle(std::string variable_name, bool isExtern);

    std::string unmangle_variable(std::string mangled);

    std::string unmangle_function(std::string mangled);

    std::string combine_names(const std::string &a, std::string b);
}
#endif //CHEESE_NAMES_H
