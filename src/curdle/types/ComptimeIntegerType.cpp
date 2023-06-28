//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/values/ComptimeFloat.h"
#include "curdle/GlobalContext.h"
#include "curdle/curdle.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/Float64Type.h"

namespace cheese::curdle {

    void ComptimeIntegerType::mark_type_references() {

    }

    ComptimeIntegerType *ComptimeIntegerType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: comptime_int")) {
            auto ref = gctx->gc.gcnew<ComptimeIntegerType>();
            gctx->cached_objects["type: comptime_int"] = ref;
            return ref;
        }
        return dynamic_cast<ComptimeIntegerType *>(gctx->cached_objects["type: comptime_int"]);
    }

    Comptimeness ComptimeIntegerType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t ComptimeIntegerType::compare(Type *other, bool implicit) {
        if (other == this) {
            return 0;
        } else if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            return 131071 - other_i->size;
        } else if (!implicit) {
            if (auto other_f = dynamic_cast<ComptimeFloat *>(other); other_f) {
                return 1;
            }
            if (auto other_f = dynamic_cast<Float64Type *>(other); other_f) {
                return 1;
            }
        }
        return -1;
    }

    std::string ComptimeIntegerType::to_string() {
        return "comptime_int";
    }

    gcref<Type> ComptimeIntegerType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            return REF(other_i);
        } else if (auto other_f = dynamic_cast<Float64Type *>(other); other_f) {
            return REF(other_f);
        } else {
            return NO_PEER;
        }
    }

    bacteria::TypePtr ComptimeIntegerType::get_bacteria_type() {
        NO_BACTERIA_TYPE(comptime_int);
    }
}