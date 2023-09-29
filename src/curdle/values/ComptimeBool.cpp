//
// Created by Lexi Allen on 9/28/2023.
//

#include "curdle/values/ComptimeBool.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/curdle.h"

namespace cheese::curdle {

    void ComptimeBool::mark_value() {

    }

    bool ComptimeBool::is_same_as(ComptimeValue *other) {
        return false;
    }

    static gcref<ComptimeValue> new_value(garbage_collector &garbageCollector, ComptimeValue *value) {
        return garbageCollector.manage(value);
    }

    // Likely need to change garbageCollector to globalContext at some point
    gcref<ComptimeValue> ComptimeBool::cast(Type *target_type, garbage_collector &garbageCollector) {
#define WHEN_TYPE_IS(type, name) if (auto name = dynamic_cast<type*>(target_type); name)
        WHEN_TYPE_IS(BooleanType, pBooleanType) {
            return new_value(garbageCollector, new ComptimeBool(value, type));
        }
        WHEN_TYPE_IS(ComptimeIntegerType, pComptimeIntegerType) {
            return new_value(garbageCollector, garbageCollector.gcnew<ComptimeInteger>(value ? 0 : 1, type));
        }
        WHEN_TYPE_IS(IntegerType, pIntegerType) {
            return new_value(garbageCollector, garbageCollector.gcnew<ComptimeInteger>(value ? 0 : 1, type));
        }
        throw CurdleError(
                "Bad Compile Time Cast: Cannot convert value of type: " + type->to_string() + " to type: " +
                target_type->to_string(), error::ErrorCode::BadComptimeCast);
    }

    std::string ComptimeBool::to_string() {
        return value ? "true" : "false";
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
ComptimeBool* rhsb = dynamic_cast<ComptimeBool*>(rhs.get());        \
if (rhsb == nullptr) throw CurdleError("Invalid Comptime Operation: Cannot " #name " a value of type " + type->to_string() + " and a value of type " + rhs->to_string(),error::ErrorCode::InvalidComptimeOperation)

    gcref<ComptimeValue> ComptimeBool::op_not(GlobalContext *gctx) {
        return gctx->gc.gcnew<ComptimeBool>(!value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_equal(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(equal);
        return gctx->gc.gcnew<ComptimeBool>(value == rhsb->value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_not_equal(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(not_equal);
        return gctx->gc.gcnew<ComptimeBool>(value != rhsb->value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_and(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(and);
        return gctx->gc.gcnew<ComptimeBool>(value && rhsb->value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_xor(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(xor);
        return gctx->gc.gcnew<ComptimeBool>(value != rhsb->value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_or(GlobalContext *gctx, ComptimeValue *other) {
        PUSH_PEER(or);
        return gctx->gc.gcnew<ComptimeBool>(value || rhsb->value, type);
    }

}