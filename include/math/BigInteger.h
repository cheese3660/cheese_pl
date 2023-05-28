//
// Created by Lexi Allen on 11/8/2022.
//

#ifndef CHEESE_BIGINTEGER_H
#define CHEESE_BIGINTEGER_H

#include <type_traits>
#include <vector>
#include <concepts>
#include <string>
#include <array>
#include <cmath>
#include <iostream>

namespace cheese::math {
    //Define a concept for the cast operator and math operators

    struct BigInteger {
        std::vector<std::uint32_t> words{}; //Stored in little endian
        bool sign{false};

        BigInteger() = default;

        BigInteger(const BigInteger &other) = default;


        explicit BigInteger(std::string_view initializer);

        template<std::unsigned_integral T>
        BigInteger(T other) {
            if constexpr (sizeof(other) <= sizeof(std::uint32_t)) {
                words.push_back(other);
            } else {
                while (other != 0) {
                    words.push_back(other & 0xfffffffful);
                    other >>= 32;
                }
            }
            sign = false;
        }


        template<std::signed_integral T>
        BigInteger(T other) {
            if (other == std::numeric_limits<T>::min()) {
                // Now we do something different
                auto max = std::numeric_limits<T>::max(); // Should be -other + 1
                auto other_v = BigInteger((std::make_unsigned_t<T>) max);
                other_v += 1;
                words = other_v.words;
                sign = true;
                return;
            }
            if (other < 0) {
                sign = true;
                other = -other;
            }
            std::make_unsigned_t<T> us = other;
            if constexpr (sizeof(us) <= sizeof(std::uint32_t)) {
                words.push_back(us);
            } else {
                while (us != 0) {
                    words.push_back(us & 0xfffffffful);
                    us >>= 32;
                }
            }
        }


        template<std::floating_point T>
        explicit BigInteger(const T other) {
            if (std::isnan(other) || std::isinf(other)) return;

            T rounded = std::abs(std::round(other));
            BigInteger multiplication = 1;
            while (rounded >= 1) {
                int mod = rounded % 10;
                rounded /= 10;
                *this += (multiplication * mod);
                multiplication *= 10;
            }
            if (other < 0) {
                sign = true;
            }
        }


        // Used in the multiplication algorithm
        BigInteger(const uint64_t base, const size_t shift) {
            for (size_t i = 0; i < shift >> 5; i++) {
                words.push_back(0);
            }
            auto remainder = shift & 0x1f;
            uint32_t low = (base << remainder) & 0xfffffffful;
            uint32_t med = (base << remainder) >> 32;
            uint32_t high = ((base >> 32) << remainder) >> 32;
            words.push_back(low);
            words.push_back(med);
            words.push_back(high);
            normalize_size();
//            words.push_back((base << remainder) & 0xfffffffful);
//            words.push_back(((static_cast<uint64_t>(base)) << remainder) >> 32);
//            normalize_size();
        }

        BigInteger twos_complement(const std::size_t normalized_size, bool negate = false) const;

        void reverse_complement();

        void normalize_size();


        //Time to do all the math operators for each type of value
        BigInteger operator+() const;

        BigInteger operator-() const;

        BigInteger operator~() const;

        BigInteger operator+(const BigInteger &other) const;

        BigInteger operator-(const BigInteger &other) const;

        BigInteger operator*(const BigInteger &other) const;

        BigInteger operator/(const BigInteger &other) const;

        BigInteger operator%(const BigInteger &other) const;

        BigInteger operator|(const BigInteger &other) const;

        BigInteger operator^(const BigInteger &other) const;

        BigInteger operator&(const BigInteger &other) const;

        BigInteger operator<<(size_t other) const;

        BigInteger operator>>(size_t other) const;

        BigInteger operator++(int);

        BigInteger &operator++();

        const BigInteger operator--(int);

        BigInteger &operator--();

        BigInteger &operator+=(const BigInteger &other);

        BigInteger &operator-=(const BigInteger &other);

        BigInteger &operator*=(const BigInteger &other);

        BigInteger &operator/=(const BigInteger &other);

        BigInteger &operator%=(const BigInteger &other);

        BigInteger &operator|=(const BigInteger &other);

        BigInteger &operator^=(const BigInteger &other);

        BigInteger &operator&=(const BigInteger &other);

        BigInteger &operator<<=(std::size_t other);

        BigInteger &operator>>=(std::size_t other);

        template<typename T>
        BigInteger operator+(const T &other) const {
            return *this + BigInteger{other};
        }

        template<typename T>
        BigInteger operator-(const T &other) const {
            return *this - BigInteger{other};
        }

        template<typename T>
        BigInteger operator*(const T &other) const {
            return *this * BigInteger{other};
        }

        template<typename T>
        BigInteger operator%(const T &other) const {
            return *this % BigInteger{other};
        }

        template<typename T>
        BigInteger operator/(const T &other) const {
            return *this / BigInteger{other};
        }

        template<typename T>
        BigInteger operator^(const T &other) const {
            return *this ^ BigInteger{other};
        }

        template<typename T>
        BigInteger operator&(const T &other) const {
            return *this & BigInteger{other};
        }


        std::strong_ordering operator<=>(const BigInteger &other) const;

        template<typename T>
        std::strong_ordering operator<=>(const T &other) const {
            BigInteger sub = *this - BigInteger{other};
            if (sub.sign) {
                return std::strong_ordering::less;
            } else {
                bool zero = true;
                for (auto x: sub.words) {
                    if (x != 0) {
                        zero = false;
                        break;
                    }
                }
                if (zero) {
                    return std::strong_ordering::equivalent;
                } else {
                    return std::strong_ordering::greater;
                }
            }
        }

        template<typename T>
        friend bool operator==(const BigInteger &a, const T &other) {
            return (a <=> other) == std::strong_ordering::equivalent;
        }

        template<typename T>
        friend bool operator!=(const BigInteger &a, const T &other) {
            return (a <=> other) != std::strong_ordering::equivalent;
        }


        template<std::unsigned_integral T>
        BigInteger &operator=(const T other) {
            return *this = BigInteger{other};
        }

        template<std::signed_integral T>
        BigInteger &operator=(const T other) {
            return *this = BigInteger{other};
        }

        template<std::floating_point T>
        BigInteger &operator=(const T other) {
            return *this = BigInteger{other};
        }

        BigInteger &operator=(const BigInteger &other) {
            if (this == &other) return *this;
            words = other.words;
            sign = other.sign;
            return *this;
        }

        template<std::unsigned_integral T>
        operator T() {
            T res = 0;
            for (uintmax_t i = 0; i < words.size(); i++) {
                res |= static_cast<T>(words[i]) << (i * 32);
            }
            if (sign) {
                res = ~res;
                res += 1;
            }
            return res;
        }

        template<std::integral T>
        operator T() const {
            auto twos = twos_complement(words.size() + 1);
            std::make_unsigned_t<T> us = 0;
            for (size_t i = 0; i < twos.words.size(); i++) {
                us |= static_cast<std::make_unsigned_t<T>>(twos.words[i]) << (i * 32);
            }
            return static_cast<T>(us);
        }

        template<std::floating_point T>
        operator T() const {
            long double shift = 4294967295;
            long double shift_mod = 1;
            T res = 0.0;
            for (unsigned int word: words) {
                res += word * shift_mod;
                shift_mod *= shift;
            }
            if (sign) {
                res *= -1;
            }
            return res;
        }

        explicit operator std::string() const;


        void set(std::size_t bit, bool value);

        [[nodiscard]] bool get(std::size_t bit) const;


        [[nodiscard]] bool zero() const;

    };


}


#endif //CHEESE_BIGINTEGER_H
