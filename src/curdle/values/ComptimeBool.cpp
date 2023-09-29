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

    gcref<ComptimeValue> ComptimeBool::op_not(GlobalContext *gctx) {
        return gctx->gc.gcnew<ComptimeBool>(!value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_equal(GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeBool>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value == rhs->value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_not_equal(GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_not_equal(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeBool>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value != rhs->value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_and(GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_and(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeBool>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value && rhs->value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_xor(GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_xor(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeBool>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value != rhs->value, type);
    }

    gcref<ComptimeValue> ComptimeBool::op_or(GlobalContext *gctx, ComptimeValue *other) {
        bool cast_self;
        auto peer = binary_peer_lhs(other->type, cast_self, gctx);
        if (cast_self) return cast(peer, gctx->gc)->op_or(gctx, other);
        auto rhs = binary_peer_rhs<ComptimeBool>(other, peer, gctx);
        return gctx->gc.gcnew<ComptimeBool>(value || rhs->value, type);
    }

}