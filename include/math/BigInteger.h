//
// Created by Lexi Allen on 11/8/2022.
//

#ifndef CHEESE_BIGINTEGER_H
#define CHEESE_BIGINTEGER_H

#include <type_traits>
#include <vector>
#include <concepts>

namespace cheese::math {
    //Define a concept for the cast operator and math operators

    class BigInteger {
        std::vector<std::uint32_t> words; //Stored in little endian




        template <typename T>
        BigInteger& operator=(const T& other) {
            if constexpr (std::is_integral_v<T>) {

            } else if constexpr (std::is_floating_point_v<T>) {

            } else {
            }
            return *this;
        }
        BigInteger& operator=(const BigInteger& other) {
            if (this == &other) return *this;
            words = other.words;
            return *this;
        }

        template <std::unsigned_integral T>
        operator T() {
            T res = 0;
        }

//        template <typename T>
//        operator T() {
//            if constexpr (std::is_integral_v<T>) {
//                T res = 0;
//                for (uintmax_t i = 0; i < words.size(); i++) {
//                    res |= static_cast<T>(words[i]) << (i * 32);
//                }
//                return res;
//            } else if constexpr (std::is_floating_point_v<T>) {
//                long double shift = 4294967295;
//                long double shift_mod = 1;
//                T res = 0.0;
//
//                for (uintmax_t i = 0; i < words.size(); i++) {
//                    res += words[i] * shift_mod;
//                    shift_mod *= shift;
//                }
//            } else {
//            }
//        }


    };
}


#endif //CHEESE_BIGINTEGER_H
