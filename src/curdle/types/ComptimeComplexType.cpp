//
// Created by Lexi Allen on 6/29/2023.
//
//
// Created by Lexi Allen on 6/29/2023.
//
#include "curdle/types/ComptimeComplexType.h"
#include "curdle/types/AnyType.h"
#include "project/GlobalContext.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/types/Float64Type.h"
#include "curdle/curdle.h"
#include "curdle/types/Complex64Type.h"
#include "curdle/values/ComptimeString.h"


namespace cheese::curdle {

    bacteria::TypePtr ComptimeComplexType::get_bacteria_type() {
        NO_BACTERIA_TYPE(comptime_complex);
    }

    void ComptimeComplexType::mark_type_references() {

    }

    ComptimeComplexType *ComptimeComplexType::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: comptime_complex")) {
            auto ref = gctx->gc.gcnew<ComptimeComplexType>();
            gctx->cached_objects["type: c64"] = ref;
            return ref;
        }
        return dynamic_cast<ComptimeComplexType *>(gctx->cached_objects["type: comptime_complex"]);
    }

    Comptimeness ComptimeComplexType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t ComptimeComplexType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto as_int = dynamic_cast<IntegerType *>(other); as_int) {
            return 131071;
        }
        if (dynamic_cast<ComptimeIntegerType *>(other) != nullptr) {
            return 131071;
        }
        if (dynamic_cast<ComptimeFloatType *>(other) != nullptr) {
            return 131071;
        }
        if (dynamic_cast<Float64Type *>(other) != nullptr) {
            return 131071 - 64;
        }
        if (dynamic_cast<Complex64Type *>(other) != nullptr) {
            return 131071 - 128;
        }
        return -1;
    }

    std::string ComptimeComplexType::to_string() {
        return "comptime_complex";
    }

    gcref<Type> ComptimeComplexType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        if (dynamic_cast<ComptimeIntegerType *>(other) != nullptr ||
            dynamic_cast<ComptimeFloatType *>(other) != nullptr) {
            return REF(this);
        }
        if (dynamic_cast<Float64Type *>(other) != nullptr || dynamic_cast<IntegerType *>(other) != nullptr ||
            dynamic_cast<Complex64Type *>(other) != nullptr) {
            return REF(Complex64Type::get(gctx));
        }
        return NO_PEER;
    }

    gcref<ComptimeValue>
    ComptimeComplexType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        INVALID_CHILD;
    }
}