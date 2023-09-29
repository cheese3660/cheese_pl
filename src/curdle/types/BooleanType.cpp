//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/BooleanType.h"
#include "curdle/types/AnyType.h"
#include "project/GlobalContext.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/types/IntegerType.h"
#include "curdle/Types/ComptimeStringType.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/curdle.h"

namespace cheese::curdle {
    bacteria::TypePtr BooleanType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        return program->get_type(bacteria::BacteriaType::Type::UnsignedInteger, 1);
    }

    void BooleanType::mark_type_references() {

    }

    BooleanType *BooleanType::get(cheese::project::GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: boolean")) {
            auto ref = gctx->gc.gcnew<BooleanType>();
            gctx->cached_objects["type: boolean"] = ref;
            return ref;
        }
        return dynamic_cast<BooleanType *>(gctx->cached_objects["type: boolean"]);
    }

    Comptimeness BooleanType::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    int32_t BooleanType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        return -1;
    }

    std::string BooleanType::to_string() {
        return "bool";
    }

    gcref<Type> BooleanType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }

    gcref<ComptimeValue> BooleanType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        if (key == "operator and") {
            // Generating the functions for operators and such, will be hell
            // Well ... actually ... we can just have a special operator type that functions as a macro,
            // And implements the functional concept
            // such that when passed in as a template,
        }
        INVALID_CHILD;
    }
}