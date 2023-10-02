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
    ArrayType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        if (key == "subtype") {}
        if (key == "rank") {}
        if (key == "dimensions") {}
        INVALID_CHILD;
    }

    bacteria::TypePtr ArrayType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        return program->get_type(bacteria::BacteriaType::Type::Array, 0, subtype->get_cached_type(program), dimensions,
                                 {}, {}, constant);
    }

    void ArrayType::mark_type_references() {
        subtype->mark();
    }

    ArrayType::ArrayType(Type *subtype, std::vector<std::size_t> dimensions, bool constant) : subtype(subtype),
                                                                                              dimensions(std::move(
                                                                                                      dimensions)),
                                                                                              constant(constant) {

    }

    Comptimeness ArrayType::get_comptimeness() {
        return subtype->get_comptimeness();
    }

    int32_t ArrayType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto as_array = dynamic_cast<ArrayType *>(other); as_array) {
            if ((!constant || (constant && as_array->constant)) && as_array->subtype->compare(subtype) == 0) {
                if (as_array->dimensions != dimensions) return -1;
                return constant == as_array->constant ? 0 : 1;
            }
        }
        if (auto as_ref = dynamic_cast<PointerType *>(other); as_ref) {
            if ((!constant || (constant && as_ref->constant)) && as_ref->subtype->compare(subtype) == 0) {
                return 1;
            }
        }
        return -1;
    }

    std::string ArrayType::to_string() {
        std::stringstream ss;
        ss << "[";
        for (int i = 0; i < dimensions.size(); i++) {
            ss << dimensions[i];
            if (i != dimensions.size() - 1) {
                ss << ",";
            }
        }
        if (constant) {
            ss << "]~";
        } else {
            ss << "]";
        }
        ss << subtype->to_string();
        return ss.str();
    }

    memory::garbage_collection::gcref<Type> ArrayType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        PEER_TYPE_CATCH_ANY();
        if (auto as_array = dynamic_cast<ArrayType *>(other); as_array) {
            if ((!constant || (constant && as_array->constant)) && as_array->subtype->compare(subtype) == 0) {
                if (as_array->dimensions == dimensions) {
                    if (constant) {
                        return REF(this);
                    } else {
                        return REF(other);
                    }
                }
            }
        }
        if (other == this) return REF(this);
        if (auto as_ref = dynamic_cast<PointerType *>(other); as_ref) {
            if ((!constant || (constant && as_ref->constant)) && as_ref->subtype->compare(subtype) == 0) {
                return REF(other);
            }
        }
        NO_PEER;
    }
}