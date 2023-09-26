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
#include "GlobalContext.h"

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
ComptimeComplex* rhsc = dynamic_cast<ComptimeComplex*>(rhs.get());        \
if (rhsc == nullptr) throw CurdleError("Invalid Comptime Operation: Cannot " #name " a value of type " + type->to_string() + " and a value of type " + rhs->to_string(),error::ErrorCode::InvalidComptimeOperation)


    gcref<ComptimeValue> ComptimeComplex::op_multiply(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(multiply);
        auto c = rhsc->a;
        auto d = rhsc->b;
        auto _a = (a * c) - (b * d);
        auto _b = (a * d) + (b * c);
        return gctx->gc.gcnew<ComptimeComplex>(_a, _b, peer);
    }

    gcref<ComptimeValue> ComptimeComplex::op_divide(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(divide);
        auto c = rhsc->a;
        auto d = rhsc->b;
        auto bottom = (c * c + d * d);
        auto _a = ((a * c) + (b * d)) / bottom;
        auto _b = ((b * c) - (a * d)) / bottom;
        return gctx->gc.gcnew<ComptimeComplex>(_a, _b, peer);
    }

    gcref<ComptimeValue> ComptimeComplex::op_add(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(add);
        return gctx->gc.gcnew<ComptimeComplex>(a + rhsc->a, b + rhsc->b, peer);
    }

    gcref<ComptimeValue> ComptimeComplex::op_subtract(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(subtract);
        return gctx->gc.gcnew<ComptimeComplex>(a - rhsc->a, b - rhsc->b, peer);
    }

    gcref<ComptimeValue> ComptimeComplex::op_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(equal);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeComplex::op_not_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(not_equal);
        NOT_IMPL;
    }
}