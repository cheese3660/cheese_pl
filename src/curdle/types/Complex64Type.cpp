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
#include "curdle/curdle.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/values/ComptimeInteger.h"
#include "GlobalContext.h"
#include <llvm/IR/Type.h>


namespace cheese::curdle {

    bacteria::TypePtr Complex64Type::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Complex64);
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
        if (key == "__name__") {
            return gctx->gc.gcnew<ComptimeString>("c64");
        }
        if (key == "__size__") {
            auto ty_ptr = get_cached_type()->get_llvm_type(gctx->machine);

            ty_ptr->dump();
        }
        throw CurdleError("key not a comptime child of type c64: " + key,
                          error::ErrorCode::InvalidSubscript);
    }
}