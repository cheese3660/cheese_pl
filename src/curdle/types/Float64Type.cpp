//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/Float64Type.h"
#include "curdle/types/AnyType.h"
#include "project/GlobalContext.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/types/Complex64Type.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/values/ComptimeFloat.h"

#include "curdle/types/ComptimeStringType.h"

#include "curdle/curdle.h"
#include <limits>

namespace cheese::curdle {

    bacteria::TypePtr Float64Type::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        return program->get_type(bacteria::BacteriaType::Type::Float64);
    }

    void Float64Type::mark_type_references() {

    }

    Float64Type *Float64Type::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: f64")) {
            auto ref = gctx->gc.gcnew<Float64Type>();
            gctx->cached_objects["type: f64"] = ref;
            return ref;
        }
        return dynamic_cast<Float64Type *>(gctx->cached_objects["type: f64"]);
    }

    Comptimeness Float64Type::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    int32_t Float64Type::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto as_int = dynamic_cast<IntegerType *>(other); as_int) {
            return 131071;
        }
        if (dynamic_cast<ComptimeIntegerType *>(other) != nullptr) {
            return 131071;
        }
        if (dynamic_cast<ComptimeFloatType *>(other) != nullptr) {
            return 131071 - 64;
        }
        return -1;
    }

    std::string Float64Type::to_string() {
        return "f64";
    }

    gcref<Type> Float64Type::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        if (dynamic_cast<Complex64Type *>(other) != nullptr) {
            return REF(other);
        }
        if (dynamic_cast<IntegerType *>(other) != nullptr ||
            dynamic_cast<ComptimeIntegerType *>(other) != nullptr ||
            dynamic_cast<ComptimeFloatType *>(other) != nullptr) {
            return REF(this);
        }
        return NO_PEER;
    }

    memory::garbage_collection::gcref<ComptimeValue>
    Float64Type::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        if (key == "zero") {
            return gctx->gc.gcnew<ComptimeFloat>(0.0, get(gctx));
        }
        if (key == "signaling_nan") {
            return gctx->gc.gcnew<ComptimeFloat>(std::numeric_limits<double>::signaling_NaN(), get(gctx));
        }
        if (key == "nan") {
            return gctx->gc.gcnew<ComptimeFloat>(std::numeric_limits<double>::quiet_NaN(), get(gctx));
        }
        if (key == "infinity") {
            return gctx->gc.gcnew<ComptimeFloat>(std::numeric_limits<double>::infinity(), get(gctx));
        }
        if (key == "epsilon") {
            return gctx->gc.gcnew<ComptimeFloat>(std::numeric_limits<double>::epsilon(), get(gctx));
        }
        if (key == "min") {
            return gctx->gc.gcnew<ComptimeFloat>(std::numeric_limits<double>::min(), get(gctx));
        }
        if (key == "denormalized_min") {
            return gctx->gc.gcnew<ComptimeFloat>(std::numeric_limits<double>::denorm_min(), get(gctx));
        }
        if (key == "max") {
            return gctx->gc.gcnew<ComptimeFloat>(std::numeric_limits<double>::max(), get(gctx));
        }
        if (key == "lowest") {
            return gctx->gc.gcnew<ComptimeFloat>(std::numeric_limits<double>::lowest(), get(gctx));
        }
        INVALID_CHILD;
    }
}