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

    int32_t TypeType::compare(Type *other) {
        return (other == this) ? 0 : -1; // Since this is a singleton, this should be all that is necessary
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

    int32_t IntegerType::compare(Type *other) {
        // Due to being a singleton for each size of integer this should work well
        if (other == this) return 0;
        // Need to add comptime integers onto this as well.
        if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            if (sign != other_i->sign) return -1;
            if (other_i->size > size) return -1;
            return (size - other_i->size);
        }
        return -1;
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

    int32_t ReferenceType::compare(Type *other) {
        if (auto other_r = dynamic_cast<ReferenceType *>(other); other_r && child->compare(other_r->child) == 0) {
            return 0;
        }
        return -1;
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

    int32_t VoidType::compare(Type *other) {
        return other == this ? 0 : -1;
    }

    bacteria::TypePtr AnyType::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }

    void AnyType::mark_type_references() {

    }

    static AnyType *at_instance;

    AnyType *AnyType::get(memory::garbage_collection::garbage_collector &gc) {
        if (at_instance == nullptr) {
            auto ref = gc.gcnew<AnyType>();
            gc.add_root_object(ref);
            at_instance = ref;
        }
        return at_instance;
    }

    Comptimeness AnyType::get_comptimeness() {
        return Comptimeness::ArgumentDepending;
    }

    int32_t AnyType::compare(Type *other) {
        return 131072;
    }

}