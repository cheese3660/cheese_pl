//
// Created by Lexi Allen on 9/30/2022.
//

#ifndef CHEESE_BASE64_H
#define CHEESE_BASE64_H

#include <cstdint>
#include <memory>
#include <string_view>

namespace cheese::compression {
    std::pair<std::uint8_t*,size_t> base64_decode(std::string_view input);
}

#endif //CHEESE_BASE64_H
