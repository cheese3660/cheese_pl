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


namespace cheese::curdle {

    bacteria::TypePtr Float64Type::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Float64);
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
}