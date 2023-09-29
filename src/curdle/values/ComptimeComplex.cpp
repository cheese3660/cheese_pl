//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/values/ComptimeComplex.h"
#include "curdle/types/ComptimeComplexType.h"
#include "curdle/types/Complex64Type.h"
#include "curdle/curdle.h"
#include "error.h"
#include "project/GlobalContext.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/values/ComptimeFloat.h"
#include "curdle/types/BooleanType.h"
#include "curdle/values/ComptimeBool.h"

namespace cheese::curdle {
    void ComptimeComplex::mark_value() {

    }

    bool ComptimeComplex::is_same_as(ComptimeValue *other) {
        if (other->type->compare(type) != 0) return false;
        if (auto as_complex = dynamic_cast<ComptimeComplex *>(other); as_complex) {
            return a == as_complex->a && b == as_complex->b;
        } else {
            return false;
        }
    }

    std::string ComptimeComplex::to_string() {
        return std::to_string(a) + "+" + std::to_string(b) + "i";
    }

    gcref<ComptimeValue> ComptimeComplex::cast(Type *target_type, garbage_collector &garbageCollector) {
        if (dynamic_cast<ComptimeComplexType *>(target_type) != nullptr || dynamic_cast<Complex64Type *>(target_type) !=
                                                                           nullptr) {
            return garbageCollector.gcnew<ComptimeComplex>(a, b, target_type);
        }
        throw CurdleError{"Cannot cast " + type->to_string() + " to " + target_type->to_string() + " at compile time",
                          error::ErrorCode::InvalidCast};
    }

    gcref<ComptimeValue> ComptimeComplex::op_unary_plus(cheese::project::GlobalContext *gctx) {
        auto &gc = gctx->gc;
        return gc.gcnew<ComptimeComplex>(a, b, type);
    }

    gcref<ComptimeValue> ComptimeComplex::op_unary_minus(cheese::project::GlobalContext *gctx) {
        auto &gc = gctx->gc;
        return gc.gcnew<ComptimeComplex>(-a, -b, type);
    }

    gcref<ComptimeValue> ComptimeComplex::op_multiply(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_multiply(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeComplex>(other, peer, gctx);
        auto c = rhs->a;
        auto d = rhs->b;
        auto _a = (a * c) - (b * d);
        auto _b = (a * d) + (b * c);
        return gctx->gc.gcnew<ComptimeComplex>(_a, _b, peer);
    }

    gcref<ComptimeValue> ComptimeComplex::op_divide(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_divide(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeComplex>(other, peer, gctx);
        auto c = rhs->a;
        auto d = rhs->b;
        auto bottom = (c * c + d * d);
        auto _a = ((a * c) + (b * d)) / bottom;
        auto _b = ((b * c) - (a * d)) / bottom;
        return gctx->gc.gcnew<ComptimeComplex>(_a, _b, peer);
    }

    gcref<ComptimeValue> ComptimeComplex::op_add(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_add(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeComplex>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeComplex>(a + rhs->a, b + rhs->b, peer);
    }

    gcref<ComptimeValue> ComptimeComplex::op_subtract(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_subtract(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeComplex>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeComplex>(a - rhs->a, b - rhs->b, peer);
    }

    gcref<ComptimeValue> ComptimeComplex::op_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeComplex>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>((a == rhs->a) && (b == rhs->b), BooleanType::get(gctx));
    }

    gcref<ComptimeValue> ComptimeComplex::op_not_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_not_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeComplex>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>((a != rhs->a) || (b != rhs->b), BooleanType::get(gctx));
    }
}