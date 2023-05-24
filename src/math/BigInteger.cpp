//
// Created by Lexi Allen on 11/9/2022.
//
#include "math/BigInteger.h"
#include <string_view>
#include <iostream>

const std::array<std::uint32_t, 256> number_values{
        {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
                0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
        }
};
namespace cheese::math {
    BigInteger::BigInteger(std::string_view initializer) {
        if (initializer.starts_with("-")) {
            sign = true;
            initializer = initializer.substr(1);
        }
        if (initializer.starts_with("0b")) {
            initializer = initializer.substr(2);
            size_t bit = 0;
            for (auto iter = initializer.rbegin(); iter != initializer.rend(); iter++) {
                auto val = *iter;
                if (bit + 1 >= 32 * words.size()) {
                    words.push_back(0);
                }
                auto word = bit / 32;
                auto sub_bit = bit % 32;
                words[word] |= number_values[val] << sub_bit;


                if (val != '_') {
                    bit++;
                }
            }
        } else if (initializer.starts_with("0o")) {
            initializer = initializer.substr(2);
            size_t bit = 0;
            for (auto iter = initializer.rbegin(); iter != initializer.rend(); iter++) {
                auto val = *iter;
                if (bit + 3 >= 32 * words.size()) {
                    words.push_back(0);
                }
                auto v = number_values[val];
                for (size_t i = 0; i < 3; i++) {
                    auto word = (i + bit) / 32;
                    auto sub_bit = (i + bit) % 32;
                    auto v2 = (v >> i) & 1;
                    words[word] |= v2 << sub_bit;
                }
                if (val != '_') {
                    bit += 3;
                }
            }
        } else if (initializer.starts_with("0x")) {
            initializer = initializer.substr(2);
            size_t bit = 0;
            for (auto iter = initializer.rbegin(); iter != initializer.rend(); iter++) {
                auto val = *iter;
                if (bit + 4 >= 32 * words.size()) {
                    words.push_back(0);
                }
                auto word = bit / 32;
                auto sub_bit = bit % 32;
                words[word] |= number_values[val] << sub_bit;
                if (val != '_') {
                    bit += 4;
                }
            }
        } else {
            bool old_sign = sign;

            *this = 0;
            for (auto ch: initializer) {
                *this *= 10;
                *this += number_values[ch];
            }

            sign = old_sign;
        }
    }


    BigInteger BigInteger::twos_complement(const std::size_t normalized_size, bool negate) const {
        BigInteger b{};
        b.words.resize(normalized_size);
        bool true_sign = negate ? !sign : sign;
        uint64_t carry = true_sign ? 1 : 0;

        for (std::size_t i = 0; i < normalized_size; i++) {
            if (i < words.size()) {
                b.words[i] = words[i];
            }
            if (true_sign) {
                b.words[i] = ~b.words[i];
            }
            if (carry > 0) {
                std::uint64_t n = static_cast<uint64_t>(b.words[i]) + carry;
                b.words[i] = n & 0xfffffffful;
                carry = n >> 32;
            }
        }
        return b;
    }


    void BigInteger::reverse_complement() {
        sign = (words[words.size() - 1] >> 31 != 0);
        if (sign) {
            uint64_t carry = 1;
            for (std::size_t i = 0; i < words.size(); i++) {
                words[i] = ~words[i];
                if (carry > 0) {
                    std::uint64_t n = static_cast<uint64_t>(words[i]) + carry;
                    words[i] = n & 0xfffffffful;
                    carry = n >> 32;
                }
            }
        }
        normalize_size();
    }

    void BigInteger::normalize_size() {
        while (words.size() > 1 && words.back() == 0) {
            words.pop_back();
        }
    }

    BigInteger BigInteger::operator+() const {
        return *this;
    }

    BigInteger BigInteger::operator-() const {
        BigInteger copy = *this;
        copy.sign = !sign;
        return copy;
    }


    BigInteger BigInteger::operator+(const BigInteger &other) const {
        std::size_t max_size = std::max(words.size(), other.words.size()) + 1;
        BigInteger a = this->twos_complement(max_size);
        BigInteger b = other.twos_complement(max_size);
        uint64_t carry = 0;
        for (std::size_t i = 0; i < max_size; i++) {
            std::uint64_t n = static_cast<uint64_t>(a.words[i]) + static_cast<uint64_t>(b.words[i]) + carry;
            a.words[i] = n & 0xfffffffful;
            carry = n >> 32;
        }
        a.reverse_complement();
        return a;
    }

    BigInteger BigInteger::operator-(const BigInteger &other) const {
        std::size_t max_size = std::max(words.size(), other.words.size()) + 1;
        BigInteger a = this->twos_complement(max_size);
        BigInteger b = other.twos_complement(max_size, true);
        uint64_t carry = 0;
        for (std::size_t i = 0; i < max_size; i++) {
            std::uint64_t n = static_cast<uint64_t>(a.words[i]) + static_cast<uint64_t>(b.words[i]) + carry;
            a.words[i] = n & 0xfffffffful;
            carry = n >> 32;
        }
        a.reverse_complement();
        return a;
    }

    BigInteger BigInteger::operator*(const BigInteger &other) const {
        BigInteger result = 0;
        for (size_t i = 0; i < words.size(); i++) {
            for (size_t j = 0; j < other.words.size(); j++) {
                size_t full_shift = i + j;
                std::uint64_t multiple = static_cast<uint64_t>(words[i]) * static_cast<uint64_t>(other.words[j]);
                BigInteger b{multiple, full_shift};
                result += b;
            }
        }
        result.sign = sign != other.sign;
        result.normalize_size();
        return result;
    }

    BigInteger BigInteger::operator/(const BigInteger &other) const {
        if (other == 0) return 0;
        BigInteger rem = 0;
        BigInteger quo = 0;
        if (other > *this) {
            return 0;
        }
        if (*this == other) {
            return 1;
        }
        size_t n = (words.size() * 32) - 1;
        while (true) {
            bool should_break = n == 0;
            rem <<= 1;
            rem.set(0, get(n));
            if (rem >= other) {
                rem -= other;
                quo.set(n, true);
            }
            if (should_break) break;
            n -= 1;
        }
        quo.sign = sign != other.sign;
        quo.normalize_size();
        return quo;
    }

    BigInteger BigInteger::operator%(const BigInteger &other) const {
        if (other == 0) return *this;
        BigInteger rem = 0;
        BigInteger quo = 0;
        if (other > *this) {
            return *this;
        }
        if (*this == other) {
            return 0;
        }
        size_t n = (words.size() * 32) - 1;
        while (true) {
            bool should_break = n == 0;
            rem <<= 1;
            rem.set(0, get(n));
            if (rem >= other) {
                rem -= other;
                quo.set(n, true);
            }
            if (should_break) break;
            n -= 1;
        }
        rem.sign = sign != other.sign;
        rem.normalize_size();
        return rem;
    }

    BigInteger BigInteger::operator|(const BigInteger &other) const {
        size_t max_size = std::max(words.size(), other.words.size());
        BigInteger a = twos_complement(max_size);
        BigInteger b = other.twos_complement(max_size);
        for (size_t i = 0; i < max_size; i++) {
            a.words[i] |= b.words[i];
        }
        a.reverse_complement();
        a.normalize_size();
        return a;
    }

    BigInteger BigInteger::operator^(const BigInteger &other) const {
        size_t max_size = std::max(words.size(), other.words.size());
        BigInteger a = twos_complement(max_size);
        BigInteger b = other.twos_complement(max_size);
        for (size_t i = 0; i < max_size; i++) {
            a.words[i] ^= b.words[i];
        }
        a.reverse_complement();
        a.normalize_size();
        return a;
    }

    BigInteger BigInteger::operator&(const BigInteger &other) const {
        size_t max_size = std::max(words.size(), other.words.size());
        BigInteger a = twos_complement(max_size);
        BigInteger b = other.twos_complement(max_size);
        for (size_t i = 0; i < max_size; i++) {
            a.words[i] |= b.words[i];
        }
        a.reverse_complement();
        a.normalize_size();
        return a;
    }

    BigInteger BigInteger::operator~() const {
        BigInteger complement = twos_complement(words.size() + 1);
        for (size_t i = 0; i < words.size() + 1; i++) {
            complement.words[i] = ~words[i];
        }
        complement.reverse_complement();
        complement.normalize_size();
        return complement;
    }

    BigInteger BigInteger::operator<<(const size_t other) const {
        return *this * BigInteger(1, other);
    }

    BigInteger BigInteger::operator>>(const size_t other) const {
        return *this / BigInteger(1, other);
    }

    const BigInteger BigInteger::operator++(int) {
        BigInteger copy = *this;
        ++*this;
        return copy;
    }

    BigInteger &BigInteger::operator++() {
        return *this += 1;
    }

    const BigInteger BigInteger::operator--(int) {
        BigInteger copy = *this;
        --*this;
        return copy;
    }

    BigInteger &BigInteger::operator--() {
        return *this -= 1;
    }

    BigInteger &BigInteger::operator+=(const BigInteger &other) {
        *this = *this + other;
        return *this;
    }

    BigInteger &BigInteger::operator-=(const BigInteger &other) {
        *this = *this - other;
        return *this;
    }

    BigInteger &BigInteger::operator*=(const BigInteger &other) {
        *this = *this * other;
        return *this;
    }

    BigInteger &BigInteger::operator/=(const BigInteger &other) {
        *this = *this / other;
        return *this;
    }

    BigInteger &BigInteger::operator%=(const BigInteger &other) {
        *this = *this % other;
        return *this;
    }

    BigInteger &BigInteger::operator|=(const BigInteger &other) {
        *this = *this | other;
        return *this;
    }

    BigInteger &BigInteger::operator^=(const BigInteger &other) {
        *this = *this ^ other;
        return *this;
    }

    BigInteger &BigInteger::operator&=(const BigInteger &other) {
        *this = *this & other;
        return *this;
    }

    BigInteger &BigInteger::operator<<=(const std::size_t other) {
        *this = *this << other;
        return *this;
    }

    BigInteger &BigInteger::operator>>=(const std::size_t other) {
        *this = *this >> other;
        return *this;
    }

    std::strong_ordering BigInteger::operator<=>(const BigInteger &other) const {
        BigInteger sub = *this - other;
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

    void BigInteger::set(std::size_t bit, bool value) {
        auto word = bit / 32;
        auto sub = bit % 32;
        while (word >= words.size()) {
            words.push_back(0);
        }
        if (value) {
            words[word] |= 1 << sub;
        } else {
            words[word] &= ~(1 << sub);
        }
    }

    bool BigInteger::get(std::size_t bit) const {
        auto word = bit / 32;
        auto sub = bit % 32;
        if (word >= words.size()) return false;
        return ((words[word] >> sub) & 1) == 1;
    }

    BigInteger::operator std::string() const {
        if (zero()) return "0";
        std::string result = "";
        BigInteger copy = *this;
        copy.sign = false;
        while (!copy.zero()) {
            char mod = copy % 10;
            copy /= 10;
            result += mod + '0';
        }
        if (sign) result += '-';
        std::reverse(result.begin(), result.end());
        return result;
    }

    bool BigInteger::zero() const {
        for (auto word: words) {
            if (word != 0) {
                return false;
            }
        }
        return true;
    }


}