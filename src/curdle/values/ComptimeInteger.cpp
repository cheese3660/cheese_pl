//
// Created by Lexi Allen on 5/28/2023.
//
#include "curdle/values/ComptimeInteger.h"
#include "curdle/GlobalContext.h"
#include "NotImplementedException.h"
#include "curdle/Type.h"
#include "typeinfo"
#include "stringutil.h"
#include "curdle/curdle.h"
#include "curdle/values/ComptimeFloat.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/Float64Type.h"

namespace cheese::curdle {
    void ComptimeInteger::mark_value() {

    }

    bool ComptimeInteger::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_int = dynamic_cast<ComptimeInteger *>(other); as_int) {
            return value == as_int->value;
        } else {
            return false;
        }
    }

    std::string ComptimeInteger::to_string() {
        return static_cast<std::string>(value);
    }

    ComptimeInteger::ComptimeInteger(const math::BigInteger &value, Type *ty) : value(value) {
        type = ty;
    }

    static gcref<ComptimeValue> new_value(garbage_collector &garbageCollector, ComptimeValue *value) {
        return garbageCollector.manage(value);
    }


    gcref<ComptimeValue> ComptimeInteger::cast(Type *target_type, garbage_collector &garbageCollector) {
#define WHEN_TYPE_IS(type, name) if (auto name = dynamic_cast<type*>(target_type); name)
        WHEN_TYPE_IS(ComptimeIntegerType, pComptimeIntegerType) {
            return new_value(garbageCollector, new ComptimeInteger(value, pComptimeIntegerType));
        }
        WHEN_TYPE_IS(IntegerType, pIntegerType) {
            // Now we must check if the value fits in that range
//            auto max = math::BigInteger(1,pIntegerType->size);
            math::BigInteger max, min;
            if (pIntegerType->sign) {
                max = math::BigInteger(1, pIntegerType->size - 1) - 1;
                min = -math::BigInteger(1, pIntegerType->size - 1);
            } else {
                max = math::BigInteger(1, pIntegerType->size) - 1;
                min = 0;
            }
            if (value < min || value > max) {
                throw InvalidCastError(
                        "Invalid Cast: cannot convert compile time known integer with value: " +
                        static_cast<std::string>(value) +
                        " to type: " + pIntegerType->to_string() + " as it cannot fit w/in range");
            }
            return new_value(garbageCollector, new ComptimeInteger(value, pIntegerType));
        }
        WHEN_TYPE_IS(Float64Type, pFloat64Type) {
            return new_value(garbageCollector, new ComptimeFloat(value, pFloat64Type));
        }
        throw CurdleError(
                "Bad Compile Time Cast: Cannot convert value of type: " + type->to_string() + " to type: " +
                target_type->to_string(), error::ErrorCode::BadComptimeCast);
#undef WHEN_TYPE_IS
    }

    static bool is_signed(Type *type) {
        if (dynamic_cast<ComptimeIntegerType *>(type)) {
            return true;
        }
        if (auto as_int = dynamic_cast<IntegerType *>(type); as_int) {
            return as_int->sign;
        }
        return false;
    }

    static bool in_range(const math::BigInteger &value, Type *type) {
        if (dynamic_cast<ComptimeIntegerType *>(type)) {
            return true;
        }
        if (auto pIntegerType = dynamic_cast<IntegerType *>(type); pIntegerType) {
            math::BigInteger max, min;
            if (pIntegerType->sign) {
                max = math::BigInteger(1, pIntegerType->size - 1) - 1;
                min = -math::BigInteger(1, pIntegerType->size - 1);
            } else {
                max = math::BigInteger(1, pIntegerType->size) - 1;
                min = 0;
            }
            return value >= min && value <= max;
        }
        return false;
    }

    static void assert_in_range(const math::BigInteger &value, Type *type) {
        if (!in_range(value, type)) {
            throw CurdleError("Invalid Comptime Operation: Operation results in value outside of range for type: " +
                              type->to_string(), error::ErrorCode::InvalidComptimeOperation);
        }
    }

    static std::uint16_t int_size(Type *type) {
        if (auto as_int = dynamic_cast<IntegerType *>(type); as_int) {
            return as_int->size;
        }
        return 0;
    }


    gcref<ComptimeValue> ComptimeInteger::op_unary_plus(GlobalContext *gctx) {
        return gctx->gc.gcnew<ComptimeInteger>(value, type);
    }

    gcref<ComptimeValue> ComptimeInteger::op_unary_minus(GlobalContext *gctx) {
        // How de we assert the value is in range and can be signed
        if (!is_signed(type)) {
            throw CurdleError(
                    "Invalid Comptime Operation: Attempting to do a unary minus on an unsigned value of type: " +
                    type->to_string(), error::ErrorCode::InvalidComptimeOperation);
        }
        auto new_value = -value;
        assert_in_range(new_value, type);
        return gctx->gc.gcnew<ComptimeInteger>(new_value, type);
    }

    gcref<ComptimeValue> ComptimeInteger::op_not(GlobalContext *gctx) {
        math::BigInteger result;
        if (is_signed(type)) {
            // -x = (not x) + 1
            // -x - 1 = not x
            result = (-value) - 1;
        } else {
            result = value;
            auto size = int_size(type);
            for (int word = 0; word < size; word += 32) {
                if (word + 32 > size) {
                    for (int bit = 0; bit < (size & 0x1f); bit++) {
                        int index = (word * 32) + size;
                        result.set(index, !result.get(index));
                    }
                } else {
                    if (word < result.words.size()) {
                        result.words[word] = ~result.words[word];
                    } else {
                        result.words.push_back(0xffffffff);
                    }
                }
            }
            result.normalize_size();
        }
        assert_in_range(result, type);
        return gctx->gc.gcnew<ComptimeInteger>(result, type);
    }

    //TODO: make the compare operation return -2 when an explicit cast would solve the problem for better error handling
#define PUSH_PEER(name) auto peer = peer_type({type, other->type}, gctx); \
auto lhs_compare = peer->compare(type);                                    \
if (lhs_compare == -1) throw CurdleError("Bad Compile Time Cast: cannot cast a value of type " + type->to_string() + " to a value of type " + peer->to_string(), error::ErrorCode::BadComptimeCast);\
if (lhs_compare != 0) return cast(peer,gctx->gc)->op_##name(gctx,other); \
gcref<ComptimeValue> rhs = {gctx->gc,nullptr};                            \
auto rhs_compare = peer->compare(other->type);                            \
if (rhs_compare == -1) throw CurdleError("Bad Compile Time Cast: cannot cast a value of type " + other->type->to_string() + " to a value of type " + peer->to_string(), error::ErrorCode::BadComptimeCast);\
if (rhs_compare != 0) rhs = other->cast(peer,gctx->gc); \
else rhs = {gctx->gc,other};                                              \
ComptimeInteger* rhsi = dynamic_cast<ComptimeInteger*>(rhs.get());        \
if (rhsi == nullptr) throw CurdleError("Invalid Comptime Operation: Cannot " #name " a value of type " + type->to_string() + " and a value of type " + rhs->to_string(),error::ErrorCode::InvalidComptimeOperation)

    gcref<ComptimeValue> ComptimeInteger::op_multiply(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(multiply);
        // So now we assume we are both of the same type
        auto result = value * rhsi->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_divide(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(divide);
        // So now we assume we are both of the same type
        auto result = value / rhsi->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_remainder(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(remainder);
        // So now we assume we are both of the same type
        auto result = value % rhsi->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_add(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(add);
        // So now we assume we are both of the same type
        auto result = value + rhsi->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_subtract(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(subtract);
        // So now we assume we are both of the same type
        auto result = value - rhsi->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_left_shift(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(left_shift);
        // So now we assume we are both of the same type
        auto result = value << rhsi->value;
        // But now instead of asserting in range, we clip it into range, if we left shift out of range, then the bits are lost
        auto sz = int_size(peer);
        if (sz > 0) {
            // Now we trim down to the amount of bits
            if (is_signed(peer)) {
                auto num_words = (sz / 32) + ((sz % 32) > 0 ? 1 : 0);
                auto two_c = result.twos_complement(num_words, false);
                bool two_c_sign = two_c.get(sz - 1);
                for (int i = sz; i < num_words * 32; i++) {
                    two_c.set(i, two_c_sign);
                }
                two_c.reverse_complement();
                result = two_c;
            } else {
                for (int i = sz; i < result.words.size() * 32; i++) {
                    result.set(i, false);
                }
                result.normalize_size();
            }
        }
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_right_shift(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(right_shift);
        // So now we assume we are both of the same type
        // This should automatically do a right shift correctly w/ sign extension if necessary
        auto result = value >> rhsi->value;
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_lesser_than(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(lesser_than);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeInteger::op_greater_than(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(greater_than);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeInteger::op_lesser_than_equal(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(lesser_than_equal);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeInteger::op_greater_than_equal(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(greater_than_equal);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeInteger::op_equal(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(equal);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeInteger::op_not_equal(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(not_equal);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeInteger::op_and(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(and);
        return gctx->gc.gcnew<ComptimeInteger>(value & rhsi->value, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_xor(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(or);
        return gctx->gc.gcnew<ComptimeInteger>(value ^ rhsi->value, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_or(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(xor);
        return gctx->gc.gcnew<ComptimeInteger>(value | rhsi->value, peer);
    }
}