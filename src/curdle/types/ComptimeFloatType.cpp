//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/curdle.h"
#include "curdle/GlobalContext.h"
#include "curdle/types/AnyType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/Float64Type.h"

namespace cheese::curdle {
    bacteria::TypePtr ComptimeFloatType::get_bacteria_type() {
        NO_BACTERIA_TYPE(comptime_float);
    }


    void ComptimeFloatType::mark_type_references() {

    }

    ComptimeFloatType *ComptimeFloatType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: comptime_float")) {
            auto ref = gctx->gc.gcnew<ComptimeFloatType>();
            gctx->cached_objects["type: comptime_float"] = ref;
            return ref;
        }
        return dynamic_cast<ComptimeFloatType *>(gctx->cached_objects["type: comptime_float"]);
    }


    Comptimeness ComptimeFloatType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t ComptimeFloatType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto as_int = dynamic_cast<IntegerType *>(other); as_int) {
            return 131071;
        }
        if (dynamic_cast<ComptimeIntegerType *>(other) != nullptr) {
            return 131071;
        }
        if (dynamic_cast<Float64Type *>(other) != nullptr) {
            return 131071 - 64;
        }
        return -1;
    }

    std::string ComptimeFloatType::to_string() {
        return "comptime_float";
    }


    gcref<Type> ComptimeFloatType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        if (dynamic_cast<ComptimeIntegerType *>(other)) {
            return REF(this);
        }
        if (dynamic_cast<Float64Type *>(other) != nullptr || dynamic_cast<IntegerType *>(other) != nullptr) {
            return REF(other);
        }
        return NO_PEER;
    }
}