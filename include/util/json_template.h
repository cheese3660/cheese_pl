//
// Created by Lexi Allen on 5/28/2023.
//

#ifndef CHEESE_JSON_TEMPLATE_H
#define CHEESE_JSON_TEMPLATE_H

#include "../../external/json.hpp"

namespace cheese::util {
    void templatify(nlohmann::json &json, const std::filesystem::path &template_path);
}


#endif //CHEESE_JSON_TEMPLATE_H
