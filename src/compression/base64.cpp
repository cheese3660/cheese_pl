//
// Created by Lexi Allen on 10/1/2022.
//
#include "compression/base64.h"
#include <stdexcept>
#include <string>

namespace cheese::compression {
    static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
    std::pair<std::uint8_t*,size_t> base64_decode(std::string_view input) {
        if (input.size() % 4 != 0) {
            throw std::runtime_error("Expected a base 64 string with an input size divisible by 4");
        }
        auto padding = std::count(input.begin(),input.end(),'=');
        size_t padding_bytes;
        if (padding == 0) {
            padding_bytes = 0;
        } else if (padding == 1) {
            padding_bytes = 2;
        } else {
            padding_bytes = 1;
        }
        auto div = (input.size()/4)*3 + padding_bytes;
        auto result = new std::uint8_t[div];
        for (size_t i = 0,j = 0; i < input.size(); i+=4, j+=3) {
            auto quartet = input.substr(i,4);
            std::uint32_t quartet_bytes = 0;
            if (quartet.contains('=')) {
                //This is a padded quartet, do the padding stuff
                if (padding == 1) {
                    auto first = base64_chars.find(quartet[0]);
                    auto second = base64_chars.find(quartet[1]);
                    auto third = base64_chars.find(quartet[2]);
                    quartet_bytes |= first << 18;
                    quartet_bytes |= second << 12;
                    quartet_bytes |= third << 6;
                    result[j] = (quartet_bytes >> 16) & 0xff;
                    result[j+1] = (quartet_bytes >> 8) & 0xff;
                } else if (padding == 2) {
                    auto first = base64_chars.find(quartet[0]);
                    auto second = base64_chars.find(quartet[1]);
                    quartet_bytes |= first << 18;
                    quartet_bytes |= second << 12;
                    result[j] = (quartet_bytes >> 16) & 0xff;
                }
            } else {
                auto first = base64_chars.find(quartet[0]);
                auto second = base64_chars.find(quartet[1]);
                auto third = base64_chars.find(quartet[2]);
                auto fourth = base64_chars.find(quartet[3]);
                quartet_bytes |= first << 18;
                quartet_bytes |= second << 12;
                quartet_bytes |= third << 6;
                quartet_bytes |= fourth;
                result[j] = (quartet_bytes >> 16) & 0xff;
                result[j+1] = (quartet_bytes >> 8) & 0xff;
                result[j+2] = (quartet_bytes) & 0xff;
            }
        }
        return {result,div};
    }
}