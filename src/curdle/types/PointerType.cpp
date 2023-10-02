//
// Created by Lexi Allen on 10/1/2023.
//
#include "curdle/Types/ArrayType.h"
#include "curdle/comptime.h"
#include "curdle/types/ComptimeStringType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/curdle.h"
#include "curdle/types/PointerType.h"
#include "curdle/types/AnyType.h"

namespace cheese::curdle {

    memory::garbage_collection::gcref<ComptimeValue>
    PointerType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        if (key == "subtype") {}
        INVALID_CHILD;
    }

    bacteria::TypePtr PointerType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        return program->get_type(bacteria::BacteriaType::Type::Pointer, 0, subtype->get_cached_type(program), {}, {},
                                 {}, constant);
    }

    void PointerType::mark_type_references() {
        subtype->mark();
    }

    PointerType::PointerType(Type *subtype, bool constant) : subtype(subtype), constant(constant) {

    }

    Comptimeness PointerType::get_comptimeness() {
        return subtype->get_comptimeness();
    }

    int32_t PointerType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto as_array = dynamic_cast<ArrayType *>(other); as_array) {
            if ((!constant || (constant && as_array->constant)) && as_array->subtype->compare(subtype) == 0) {
                return 1;
            }
        }
        if (auto as_ref = dynamic_cast<PointerType *>(other); as_ref) {
            if ((!constant || (constant && as_ref->constant)) && as_ref->subtype->compare(subtype) == 0) {
                return (constant == as_ref->constant) ? 0 : 1;
            }
        }
        return -1;
    }

    std::string PointerType::to_string() {
        return "[?]" + subtype->to_string();
    }

    memory::garbage_collection::gcref<Type> PointerType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        PEER_TYPE_CATCH_ANY();
        if (other == this) return REF(this);
        if (auto as_ref = dynamic_cast<ArrayType *>(other); as_ref) {
            if ((!constant || (constant == as_ref->constant)) && as_ref->subtype->compare(subtype) == 0) {
                return REF(this);
            }
        }
        NO_PEER;
    }
}