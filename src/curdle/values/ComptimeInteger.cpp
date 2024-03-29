//
// Created by Lexi Allen on 5/28/2023.
//
#include "curdle/values/ComptimeInteger.h"
#include "project/GlobalContext.h"
#include "NotImplementedException.h"
#include "curdle/Type.h"
#include "typeinfo"
#include "stringutil.h"
#include "curdle/curdle.h"
#include "curdle/values/ComptimeFloat.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/Float64Type.h"
#include "curdle/values/ComptimeComplex.h"
#include "curdle/types/Complex64Type.h"
#include "curdle/types/BooleanType.h"
#include "curdle/values/ComptimeBool.h"

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

        if (auto as_complex = dynamic_cast<Complex64Type *>(target_type); as_complex) {
            return garbageCollector.gcnew<ComptimeComplex>(value, 0.0, target_type);
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
        WHEN_TYPE_IS(BooleanType, pBooleanType) {
            return new_value(garbageCollector, new ComptimeBool(value != 0, pBooleanType));
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


    gcref<ComptimeValue> ComptimeInteger::op_unary_plus(cheese::project::GlobalContext *gctx) {
        return gctx->gc.gcnew<ComptimeInteger>(value, type);
    }

    gcref<ComptimeValue> ComptimeInteger::op_unary_minus(cheese::project::GlobalContext *gctx) {
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

    gcref<ComptimeValue> ComptimeInteger::op_not(cheese::project::GlobalContext *gctx) {
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


    gcref<ComptimeValue> ComptimeInteger::op_multiply(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_multiply(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        // So now we assume we are both of the same type
        auto result = value * rhs->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_divide(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_divide(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        // So now we assume we are both of the same type
        auto result = value / rhs->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_remainder(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_remainder(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        // So now we assume we are both of the same type
        auto result = value % rhs->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_add(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_add(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        // So now we assume we are both of the same type
        auto result = value + rhs->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_subtract(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_subtract(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        // So now we assume we are both of the same type
        auto result = value - rhs->value;
        assert_in_range(result, peer);
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_left_shift(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_left_shift(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        // So now we assume we are both of the same type
        auto result = value << rhs->value;
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

    gcref<ComptimeValue> ComptimeInteger::op_right_shift(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_right_shift(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        // So now we assume we are both of the same type
        // This should automatically do a right shift correctly w/ sign extension if necessary
        auto result = value >> rhs->value;
        return gctx->gc.gcnew<ComptimeInteger>(result, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_lesser_than(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_lesser_than(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value < rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue> ComptimeInteger::op_greater_than(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_greater_than(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value > rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue>
    ComptimeInteger::op_lesser_than_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_lesser_than_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value <= rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue>
    ComptimeInteger::op_greater_than_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_greater_than_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value >= rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue> ComptimeInteger::op_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value == rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue> ComptimeInteger::op_not_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_not_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value != rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue> ComptimeInteger::op_and(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_and(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeInteger>(value & rhs->value, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_xor(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_xor(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeInteger>(value ^ rhs->value, peer);
    }

    gcref<ComptimeValue> ComptimeInteger::op_or(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_or(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeInteger>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeInteger>(value | rhs->value, peer);
    }
}