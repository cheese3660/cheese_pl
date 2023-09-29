//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/ComptimeFloat.h"
#include "curdle/curdle.h"
#include "error.h"
#include "curdle/types/Float64Type.h"
#include "curdle/types/Complex64Type.h"
#include "curdle/values/ComptimeComplex.h"
#include "project/GlobalContext.h"
#include "curdle/types/BooleanType.h"
#include "curdle/values/ComptimeBool.h"
#include "curdle/types/ComptimeComplexType.h"

namespace cheese::curdle {
    void ComptimeFloat::mark_value() {

    }

    bool ComptimeFloat::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_float = dynamic_cast<ComptimeFloat *>(other); as_float) {
            return value == as_float->value;
        } else {
            return false;
        }
    }

    std::string ComptimeFloat::to_string() {
        return std::to_string(value);
    }

    gcref<ComptimeValue> ComptimeFloat::cast(Type *target_type, garbage_collector &garbageCollector) {
        if (type->compare(target_type) == 0) return gcref{garbageCollector, this};
        if (auto as_f64 = dynamic_cast<Float64Type *>(target_type); as_f64) {
            return garbageCollector.gcnew<ComptimeFloat>(value, target_type);
        }
        if (auto as_complex64 = dynamic_cast<Complex64Type *>(target_type); as_complex64) {
            return garbageCollector.gcnew<ComptimeComplex>(value, 0.0, target_type);
        }
        if (auto as_comptime_complex = dynamic_cast<ComptimeComplexType *>(target_type); as_comptime_complex) {
            return garbageCollector.gcnew<ComptimeComplex>(value, 0.0, target_type);
        }
        throw CurdleError{"Cannot cast " + type->to_string() + " to " + target_type->to_string() + " at compile time",
                          error::ErrorCode::InvalidCast};
    }

    gcref<ComptimeValue> ComptimeFloat::op_unary_plus(cheese::project::GlobalContext *gctx) {
        return gctx->gc.gcnew<ComptimeFloat>(value, type);
    }

    gcref<ComptimeValue> ComptimeFloat::op_unary_minus(cheese::project::GlobalContext *gctx) {
        return gctx->gc.gcnew<ComptimeFloat>(-value, type);
    }

    gcref<ComptimeValue> ComptimeFloat::op_multiply(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_multiply(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeFloat>(value * rhs->value, peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_divide(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_divide(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeFloat>(value / rhs->value, peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_remainder(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_remainder(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeFloat>(std::remainder(value, rhs->value), peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_add(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_add(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeFloat>(value + rhs->value, peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_subtract(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_subtract(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeFloat>(value - rhs->value, peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_lesser_than(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_lesser_than(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value < rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue> ComptimeFloat::op_greater_than(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_greater_than(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value > rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue>
    ComptimeFloat::op_lesser_than_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_lesser_than_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value <= rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue>
    ComptimeFloat::op_greater_than_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_greater_than_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value >= rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue> ComptimeFloat::op_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value == rhs->value, BooleanType::get(gctx));
    }

    gcref<ComptimeValue> ComptimeFloat::op_not_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_not_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeFloat>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value <= rhs->value, BooleanType::get(gctx));
    }

}