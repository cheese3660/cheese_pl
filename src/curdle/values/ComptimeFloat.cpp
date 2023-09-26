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
#include "GlobalContext.h"


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
        if (auto as_complex = dynamic_cast<Complex64Type *>(target_type); as_complex) {
            return garbageCollector.gcnew<ComptimeComplex>(value, 0.0, target_type);
        }
        throw CurdleError{"Cannot cast " + type->to_string() + " to " + target_type->to_string() + " at compile time",
                          error::ErrorCode::InvalidCast};
    }

#define PUSH_PEER(name) auto peer = peer_type({type, other->type}, gctx); \
auto lhs_compare = peer->compare(type);                                    \
if (lhs_compare == -1) throw CurdleError("Bad Compile Time Cast: cannot cast a value of type " + type->to_string() + " to a value of type " + peer->to_string(), error::ErrorCode::BadComptimeCast);\
if (lhs_compare != 0) return cast(peer,gctx->gc)->op_##name(gctx,other); \
gcref<ComptimeValue> rhs = {gctx->gc,nullptr};                            \
auto rhs_compare = peer->compare(other->type);                            \
if (rhs_compare == -1) throw CurdleError("Bad Compile Time Cast: cannot cast a value of type " + other->type->to_string() + " to a value of type " + peer->to_string(), error::ErrorCode::BadComptimeCast);\
if (rhs_compare != 0) rhs = other->cast(peer,gctx->gc); \
else rhs = {gctx->gc,other};                                              \
ComptimeFloat* rhsf = dynamic_cast<ComptimeFloat*>(rhs.get());        \
if (rhsf == nullptr) throw CurdleError("Invalid Comptime Operation: Cannot " #name " a value of type " + type->to_string() + " and a value of type " + rhs->to_string(),error::ErrorCode::InvalidComptimeOperation)

    gcref<ComptimeValue> ComptimeFloat::op_unary_plus(cheese::project::GlobalContext *gctx) {
        return gctx->gc.gcnew<ComptimeFloat>(value, type);
    }

    gcref<ComptimeValue> ComptimeFloat::op_unary_minus(cheese::project::GlobalContext *gctx) {
        return gctx->gc.gcnew<ComptimeFloat>(-value, type);
    }

    gcref<ComptimeValue> ComptimeFloat::op_multiply(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(multiply);
        return gctx->gc.gcnew<ComptimeFloat>(value * rhsf->value, peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_divide(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(divide);
        return gctx->gc.gcnew<ComptimeFloat>(value / rhsf->value, peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_remainder(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(remainder);
        return gctx->gc.gcnew<ComptimeFloat>(std::remainder(value, rhsf->value), peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_add(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(add);
        return gctx->gc.gcnew<ComptimeFloat>(value + rhsf->value, peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_subtract(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(subtract);
        return gctx->gc.gcnew<ComptimeFloat>(value - rhsf->value, peer);
    }

    gcref<ComptimeValue> ComptimeFloat::op_lesser_than(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(lesser_than);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeFloat::op_greater_than(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(greater_than);
        NOT_IMPL;
    }

    gcref<ComptimeValue>
    ComptimeFloat::op_lesser_than_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(lesser_than_equal);
        NOT_IMPL;
    }

    gcref<ComptimeValue>
    ComptimeFloat::op_greater_than_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(greater_than_equal);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeFloat::op_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(equal);
        NOT_IMPL;
    }

    gcref<ComptimeValue> ComptimeFloat::op_not_equal(cheese::project::GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(not_equal);
        NOT_IMPL;
    }

}