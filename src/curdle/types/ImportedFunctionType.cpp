//
// Created by Lexi Allen on 6/28/2023.
//
//
// Created by Lexi Allen on 9/29/2023.
//
#include "curdle/types/ImportedFunctionType.h"
#include "curdle/types/AnyType.h"
#include "curdle/types/ReferenceType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/types/ComptimeStringType.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/curdle.h"
#include "curdle/values/ComptimeType.h"
#include "curdle/types/TypeType.h"

namespace cheese::curdle {

    bacteria::TypePtr ImportedFunctionType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        NO_BACTERIA_TYPE;
    }

    void ImportedFunctionType::mark_type_references() {
        return_type->mark();
        for (const auto child: argument_types) {
            child->mark();
        }
    }

    Comptimeness ImportedFunctionType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t ImportedFunctionType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto otherPtr = dynamic_cast<ImportedFunctionType *>(other); otherPtr) {
            if (return_type->compare(otherPtr->return_type) == 0) {
                if (argument_types.size() == otherPtr->argument_types.size()) {
                    for (int i = 0; i < argument_types.size(); i++) {
                        if (argument_types[i]->compare(otherPtr->argument_types[i]) != 0) return -1;
                    }
                    return 0;
                }
            }
        }
        return -1;
    }

    memory::garbage_collection::gcref<Type>
    ImportedFunctionType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        PEER_TYPE_CATCH_ANY();
        // If the other type is an opaque pointer maybe I will allow conversion
        if (compare(other, false) == 0) return REF(this);
        NO_PEER;
    }

    memory::garbage_collection::gcref<ComptimeValue>
    ImportedFunctionType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        if (key == "return_type") {
            return gctx->gc.gcnew<ComptimeType>(gctx, return_type);
        }
        if (key == "argument_types") {
            // TODO: Return a cached array of argument types
        }

        INVALID_CHILD;
    }

    std::string ImportedFunctionType::to_string() {
        std::stringstream ss{};
        ss << "fn(";
        for (int i = 0; i < argument_types.size(); i++) {
            ss << argument_types[i]->to_string();
            if (i < argument_types.size() - 1) {
                ss << ",";
            }
        }
        ss << ")=>" << return_type->to_string() + " import";
        return ss.str();
    }

    ImportedFunctionType::ImportedFunctionType(Type *returnType, std::vector<Type *> argumentTypes) : return_type(
            returnType), argument_types(argumentTypes) {

    }
}