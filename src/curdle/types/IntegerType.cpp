//
// Created by Lexi Allen on 6/28/2023.
//
#include "curdle/types/IntegerType.h"
#include "project/GlobalContext.h"
#include "curdle/types/ComptimeIntegerType.h"
#include "curdle/types/AnyType.h"
#include "curdle/types/Float64Type.h"
#include "curdle/types/ComptimeFloatType.h"
#include "curdle/types/ComptimeComplexType.h"
#include "curdle/types/Complex64Type.h"
#include "curdle/comptime.h"
#include "curdle/curdle.h"
#include "curdle/values/ComptimeString.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/types/ComptimeStringType.h"

namespace cheese::curdle {
    IntegerType *IntegerType::get(cheese::project::GlobalContext *gctx, bool sign, std::uint16_t size) {
        std::string name = std::string("type: ") + (sign ? "i" : "u") + std::to_string(size);
        if (!gctx->cached_objects.contains(name)) {
            auto ref = gctx->gc.gcnew<IntegerType>(sign, size);
            gctx->cached_objects[name] = ref;
            return ref;
        }
        return dynamic_cast<IntegerType *>(gctx->cached_objects[name]);
    }

    bacteria::TypePtr IntegerType::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        return program->get_type(
                sign ? bacteria::BacteriaType::Type::SignedInteger : bacteria::BacteriaType::Type::UnsignedInteger,
                size);
    }

    void IntegerType::mark_type_references() {

    }

    Comptimeness IntegerType::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    int32_t IntegerType::compare(Type *other, bool implicit) {
        // Due to being a singleton for each size of integer this should work well
        if (other == this) return 0;
        // Need to add comptime integers onto this as well.
        if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            if (implicit && sign != other_i->sign) return -1;
            if (implicit && other_i->size > size) return -1;
            return implicit ? (size - other_i->size) : 2;
        }
        if (auto other_c = dynamic_cast<ComptimeIntegerType *>(other); other_c) {
            return 131071 - ((size * 2) + (sign ? 1 : 0));
        }
        if (!implicit) {
            if (auto other_f = dynamic_cast<Float64Type *>(other); other_f) {
                return 1;
            }
            if (auto other_f = dynamic_cast<ComptimeFloatType *>(other); other_f) {
                return 1;
            }
            if (dynamic_cast<ComptimeComplexType *>(other) != nullptr ||
                dynamic_cast<Complex64Type *>(other) != nullptr) {
                return 1;
            }
        }
        return -1;
    }

    std::string IntegerType::to_string() {
        return (sign ? "i" : "u") + std::to_string(size);
    }

    gcref<Type> IntegerType::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        if (auto other_c = dynamic_cast<ComptimeIntegerType *>(other); other_c) {
            return REF(this);
        }
        if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            auto new_sign = sign || other_i->sign;
            auto new_size = std::max(size, other_i->size);
            return REF(IntegerType::get(gctx, new_sign, new_size));
        }
        if (dynamic_cast<Float64Type *>(other) != nullptr) {
            return REF(other);
        }
        if (dynamic_cast<ComptimeFloatType *>(other) != nullptr) {
            return REF(Float64Type::get(gctx));
        } else if (dynamic_cast<ComptimeComplexType *>(other) != nullptr || dynamic_cast<Complex64Type *>(other)) {
            return REF(Complex64Type::get(gctx));
        }
        return NO_PEER;
    }

    memory::garbage_collection::gcref<ComptimeValue>
    IntegerType::get_child_comptime(std::string key, cheese::project::GlobalContext *gctx) {
        CATCH_DUNDER_NAME;
        CATCH_DUNDER_SIZE;
        if (key == "zero") {
            return gctx->gc.gcnew<ComptimeInteger>(0, get(gctx, sign, size));
        }
        if (key == "signed") {

        }
        INVALID_CHILD;
    }
}