//
// Created by Lexi Allen on 6/29/2023.
//
#include "curdle/types/Complex64Type.h"
#include "curdle/types/AnyType.h"
#include "project/GlobalContext.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/types/Float64Type.h"
#include "curdle/types/ComptimeComplexType.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/types/IntegerType.h"
#include "curdle/Types/ComptimeStringType.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/curdle.h"
#include "curdle/values/ComptimeComplex.h"
#include <llvm/IR/Type.h>


namespace cheese::curdle {

    bacteria::TypePtr Complex64Type::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        return program->get_type(bacteria::BacteriaType::Type::Complex64);
    }

    void Complex64Type::mark_type_references() {

    }

    Complex64Type *Complex64Type::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: c64")) {
            auto ref = gctx->gc.gcnew<Complex64Type>();
            gctx->cached_objects["type: c64"] = ref;
            return ref;
        }
        return dynamic_cast<Complex64Type *>(gctx->cached_objects["type: c64"]);
    }

    Comptimeness Complex64Type::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    int32_t Complex64Type::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto as_int = dynamic_cast<IntegerType *>(other); as_int) {
            return 131071;
        }
        if (dynamic_cast<ComptimeIntegerType *>(other) != nullptr) {
            return 131071;
        }
        if (dynamic_cast<ComptimeFloatType *>(other) != nullptr || dynamic_cast<Float64Type *>(other) != nullptr) {
            return 131071 - 64;
        }
        if (dynamic_cast<ComptimeComplexType *>(other) != nullptr) {
            return 131071 - 128;
        }

        return -1;
    }

    std::string Complex64Type::to_string() {
        return "c64";
    }

    gcref<Type> Complex64Type::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        if (dynamic_cast<IntegerType *>(other) != nullptr || dynamic_cast<ComptimeIntegerType *>(other) != nullptr ||
            dynamic_cast<ComptimeFloatType *>(other) != nullptr || dynamic_cast<Float64Type *>(other) != nullptr) {
            return REF(this);
        }
        if (dynamic_cast<ComptimeComplexType *>(other) != nullptr) {
            return REF(this);
        }
        return NO_PEER;
    }

    gcref<ComptimeValue> Complex64Type::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        if (key == "zero") {
            return gctx->gc.gcnew<ComptimeComplex>(0.0, 0.0, get(gctx));
        }
        INVALID_CHILD;
    }
}