//
// Created by Lexi Allen on 4/1/2023.
//

#include "curdle/Type.h"

namespace cheese::curdle {

    bacteria::TypePtr TypeType::get_bacteria_type() {
        // This should actually throw an error if attempting to convert this to a runtime (bacteria) type
        return {};
    }

    void TypeType::mark_type_references() {

    }

    static TypeType *tt_instance;

    TypeType *TypeType::get(memory::garbage_collection::garbage_collector &gc) {
        if (tt_instance == nullptr) {
            auto ref = gc.gcnew<TypeType>();
            gc.add_root_object(ref);
            tt_instance = ref;
        }
        return tt_instance;
    }

    Comptimeness TypeType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    void Type::mark_references() {
        for (auto mixin: mixins) {
            mixin->mark();
        }
        mark_type_references();
    }

    static std::map<std::uint16_t, IntegerType *> signed_instances;
    static std::map<std::uint16_t, IntegerType *> unsigned_instances;

    IntegerType *IntegerType::get(memory::garbage_collection::garbage_collector &gc, bool sign, std::uint16_t size) {
        if (sign) {
            if (signed_instances.contains(size)) {
                return signed_instances[size];
            }
            return gc.gcnew<IntegerType>(gc, sign, size);
        } else {
            if (unsigned_instances.contains(size)) {
                return unsigned_instances[size];
            }
            return gc.gcnew<IntegerType>(gc, sign, size);
        }
    }

    bacteria::TypePtr IntegerType::get_bacteria_type() {
        return {};
    }

    void IntegerType::mark_type_references() {

    }

    IntegerType::IntegerType(memory::garbage_collection::garbage_collector &gc, bool sn, std::uint16_t sz) {
        sign = sn;
        size = sz;
        if (sign) {
            signed_instances[sz] = this;
        } else {
            unsigned_instances[sz] = this;
        }
        gc.add_root_object(this);
    }

    Comptimeness IntegerType::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    bacteria::TypePtr ReferenceType::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }

    void ReferenceType::mark_type_references() {
        child->mark();
    }

    Comptimeness ReferenceType::get_comptimeness() {
        return child->get_comptimeness();
    }

    bacteria::TypePtr VoidType::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }

    void VoidType::mark_type_references() {

    }

    static VoidType *vt_instance;

    VoidType *VoidType::get(memory::garbage_collection::garbage_collector &gc) {
        if (vt_instance == nullptr) {
            auto ref = gc.gcnew<VoidType>();
            gc.add_root_object(ref);
            vt_instance = ref;
        }
        return vt_instance;
    }

    Comptimeness VoidType::get_comptimeness() {
        return Comptimeness::Runtime;
    }
}