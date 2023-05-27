//
// Created by Lexi Allen on 4/1/2023.
//

#include "curdle/Type.h"
#include "curdle/comptime.h"
#include "error.h"
#include "curdle/curdle.h"
#include <typeinfo>

namespace cheese::curdle {
#define PEER_TYPE_CATCH_ANY() if (dynamic_cast<AnyType*>(other)) return this

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

    int32_t TypeType::compare(Type *other, bool implicit) {
        return (other == this) ? 0 : -1; // Since this is a singleton, this should be all that is necessary
    }

    std::string TypeType::to_string() {
        return "type";
    }

    Type *TypeType::peer(Type *other, garbage_collector &gc) {
        PEER_TYPE_CATCH_ANY();
        return nullptr;
    }

    void Type::mark_references() {
        for (auto mixin: mixins) {
            mixin->mark();
        }
        mark_type_references();
    }

    bacteria::TypePtr Type::get_cached_type() {
        if (cached_type) {
            return cached_type;
        } else {
            cached_type = std::move(get_bacteria_type());
            return cached_type;
        }
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
        if (sign) {
            return std::shared_ptr<bacteria::BacteriaType>(
                    new bacteria::BacteriaType(bacteria::BacteriaType::Type::SignedInteger, size));
        } else {
            return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::UnsignedInteger, size);
        }
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
        }
        return -1;
    }

    std::string IntegerType::to_string() {
        return (sign ? "i" : "u") + std::to_string(size);
    }

    Type *IntegerType::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        PEER_TYPE_CATCH_ANY();
        if (auto other_c = dynamic_cast<ComptimeIntegerType *>(other); other_c) {
            return this;
        }
        if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            auto new_sign = sign || other_i->sign;
            auto new_size = std::max(size, other_i->size);
            return IntegerType::get(gc, new_sign, new_size);
        }
        return nullptr;
    }

    bacteria::TypePtr ReferenceType::get_bacteria_type() {
        // Weak reference generation will be done at the structure level
        auto child_type = child->get_cached_type();
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Reference, 0, child_type);
    }

    void ReferenceType::mark_type_references() {
        child->mark();
    }

    Comptimeness ReferenceType::get_comptimeness() {
        return child->get_comptimeness();
    }

    // This kinda casting has to be specifically done w/ a "possible rvalue address" in bacteria which is &&(value)
    // Rather than a regular address of operator which is &value
    // But on a known lvalue the possible rvalue address gets converted into an lvalue in second stage lowering
    int32_t ReferenceType::compare(Type *other, bool implicit) {
        if (auto other_r = dynamic_cast<ReferenceType *>(other); other_r && child->compare(other_r->child) == 0) {
            return 0;
        }
        auto child_comparison = child->compare(other);
        if (child_comparison != -1) {
            return child_comparison + 1;
        }
        return -1;
    }

    std::string ReferenceType::to_string() {
        return (constant ? "*~" : "*") + child->to_string();
    }

    Type *ReferenceType::peer(Type *other, garbage_collector &gc) {
        if (compare(other) == 0) return this;
        PEER_TYPE_CATCH_ANY();
        return nullptr;
    }

    bacteria::TypePtr VoidType::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Void);
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

    int32_t VoidType::compare(Type *other, bool implicit) {
        return other == this ? 0 : -1;
    }

    std::string VoidType::to_string() {
        return "void";
    }

    Type *VoidType::peer(Type *other, garbage_collector &gc) {
        PEER_TYPE_CATCH_ANY();
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

    int32_t AnyType::compare(Type *other, bool implicit) {
        return 131072;
    }

    std::string AnyType::to_string() {
        return "any";
    }

    Type *AnyType::peer(Type *other, garbage_collector &gc) {
        return other;
    }

    bacteria::TypePtr FunctionTemplateType::get_bacteria_type() {
        return {};
    }

    void FunctionTemplateType::mark_type_references() {
    }

    static FunctionTemplateType *ftt_instance;

    FunctionTemplateType *FunctionTemplateType::get(memory::garbage_collection::garbage_collector &gc) {
        if (ftt_instance == nullptr) {
            auto ref = gc.gcnew<FunctionTemplateType>();
            gc.add_root_object(ref);
            ftt_instance = ref;
        }
        return ftt_instance;
    }

    Comptimeness FunctionTemplateType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t FunctionTemplateType::compare(Type *other, bool implicit) {
        return other == this ? 0 : -1;
    }

    std::string FunctionTemplateType::to_string() {
        return "$FunctionSet";
    }

    Type *FunctionTemplateType::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        PEER_TYPE_CATCH_ANY();
        return other;
    }

    bacteria::TypePtr ComptimeIntegerType::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }

    static ComptimeIntegerType *cit_instance;

    void ComptimeIntegerType::mark_type_references() {

    }

    ComptimeIntegerType *ComptimeIntegerType::get(memory::garbage_collection::garbage_collector &gc) {
        if (cit_instance == nullptr) {
            auto ref = gc.gcnew<ComptimeIntegerType>();
            gc.add_root_object(ref);
            cit_instance = ref;
        }
        return cit_instance;
    }

    Comptimeness ComptimeIntegerType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t ComptimeIntegerType::compare(Type *other, bool implicit) {
        if (other == this) {
            return 0;
        } else if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            return 131071 - other_i->size;
        } else if (!implicit) {
            if (auto other_f = dynamic_cast<ComptimeFloat *>(other); other_f) {
                return 1;
            }
            if (auto other_f = dynamic_cast<Float64Type *>(other); other_f) {
                return 1;
            }
        }
        return -1;
    }

    std::string ComptimeIntegerType::to_string() {
        return "comptime_int";
    }

    Type *ComptimeIntegerType::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            return other_i;
        } else if (auto other_f = dynamic_cast<Float64Type *>(other); other_f) {
            return other_f;
        } else {
            return nullptr;
        }
    }

    bacteria::TypePtr BooleanType::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::UnsignedInteger, 1);
    }

    void BooleanType::mark_type_references() {

    }

    BooleanType *bt_instance;

    BooleanType *BooleanType::get(garbage_collector &gc) {
        if (bt_instance == nullptr) {
            auto ref = gc.gcnew<BooleanType>();
            gc.add_root_object(ref);
            bt_instance = ref;
        }
        return bt_instance;
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

    Type *BooleanType::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        PEER_TYPE_CATCH_ANY();
        return nullptr;
    }

    BuiltinReferenceType *brt_instance;

    bacteria::TypePtr BuiltinReferenceType::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }

    void BuiltinReferenceType::mark_type_references() {

    }

    BuiltinReferenceType *BuiltinReferenceType::get(garbage_collector &gc) {
        if (brt_instance == nullptr) {
            auto ref = gc.gcnew<BuiltinReferenceType>();
            gc.add_root_object(ref);
            brt_instance = ref;
        }
        return brt_instance;
    }


    Comptimeness BuiltinReferenceType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t BuiltinReferenceType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        return -1;
    }

    std::string BuiltinReferenceType::to_string() {
        return "$BuiltinReference";
    }

    Type *BuiltinReferenceType::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        PEER_TYPE_CATCH_ANY();
        return nullptr;
    }

    NoReturnType *nrt_instance;

    NoReturnType *NoReturnType::get(garbage_collector &gc) {
        if (nrt_instance == nullptr) {
            auto ref = gc.gcnew<NoReturnType>();
            gc.add_root_object(ref);
            nrt_instance = ref;
        }
        return nrt_instance;
    }

    bacteria::TypePtr NoReturnType::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }

    void NoReturnType::mark_type_references() {

    }

    Comptimeness NoReturnType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t NoReturnType::compare(Type *other, bool implicit) {
        return 0; // No return can "cast" to anything
    }

    std::string NoReturnType::to_string() {
        return "noreturn";
    }

    Type *NoReturnType::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        PEER_TYPE_CATCH_ANY();
        return other;
    }

    bacteria::TypePtr Float64Type::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Float64);
    }

    void Float64Type::mark_type_references() {

    }

    Float64Type *f64t_instance;

    Float64Type *Float64Type::get(garbage_collector &gc) {
        if (f64t_instance == nullptr) {
            auto ref = gc.gcnew<Float64Type>();
            gc.add_root_object(ref);
            f64t_instance = ref;
        }
        return f64t_instance;
    }

    Comptimeness Float64Type::get_comptimeness() {
        return Comptimeness::Runtime;
    }

    int32_t Float64Type::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto as_int = dynamic_cast<IntegerType *>(other); as_int) {
            return 131071;
        }
        if (dynamic_cast<ComptimeIntegerType *>(other)) {
            return 131071;
        }
        if (dynamic_cast<ComptimeFloatType *>(other)) {
            return 131071 - 64;
        }
        return -1;
    }

    std::string Float64Type::to_string() {
        return "f64";
    }

    Type *Float64Type::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        PEER_TYPE_CATCH_ANY();
        if (dynamic_cast<IntegerType *>(other) || dynamic_cast<ComptimeIntegerType *>(other) ||
            dynamic_cast<ComptimeFloatType *>(other)) {
            return this;
        }
        return nullptr;
    }

    bacteria::TypePtr ComptimeFloatType::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }


    void ComptimeFloatType::mark_type_references() {

    }

    ComptimeFloatType *cft_instance;

    ComptimeFloatType *ComptimeFloatType::get(garbage_collector &gc) {
        if (cft_instance == nullptr) {
            auto ref = gc.gcnew<ComptimeFloatType>();
            gc.add_root_object(ref);
            cft_instance = ref;
        }
        return cft_instance;
    }


    Comptimeness ComptimeFloatType::get_comptimeness() {
        return Comptimeness::Comptime;
    }

    int32_t ComptimeFloatType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto as_int = dynamic_cast<IntegerType *>(other); as_int) {
            return 131071;
        }
        if (dynamic_cast<ComptimeIntegerType *>(other)) {
            return 131071;
        }
        if (dynamic_cast<Float64Type *>(other)) {
            return 131071 - 64;
        }
        return -1;
    }

    std::string ComptimeFloatType::to_string() {
        return "comptime_float";
    }


    Type *ComptimeFloatType::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        PEER_TYPE_CATCH_ANY();
        if (dynamic_cast<ComptimeIntegerType *>(other)) {
            return this;
        }
        if (dynamic_cast<Float64Type *>(other) || dynamic_cast<IntegerType *>(other)) {
            return other;
        }
        return nullptr;
    }


    bacteria::TypePtr ErrorType::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }

    void ErrorType::mark_type_references() {

    }

    Comptimeness ErrorType::get_comptimeness() {
        return Comptimeness::Comptime;
    }


    int32_t ErrorType::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        return -1;
    }

    // Lets always coalesce error types w/ peer types
    Type *ErrorType::peer(Type *other, garbage_collector &gc) {
        if (other == this) return this;
        PEER_TYPE_CATCH_ANY();
        return other;
    }

    std::string ErrorType::to_string() {
        return "ERROR!";
    }


    ErrorType *et_instance;

    ErrorType *ErrorType::get(garbage_collector &gc) {
        if (et_instance == nullptr) {
            auto ref = gc.gcnew<ErrorType>();
            gc.add_root_object(ref);
            et_instance = ref;
        }
        return et_instance;
    }


    std::string get_peer_type_list(std::vector<Type *> types) {
        std::stringstream ss{};
        for (int i = 0; i < types.size(); i++) {
            ss << types[i]->to_string();
            if (i < types.size() - 1) {
                ss << ", ";
            }
            if (i > 0 and i == types.size() - 2) {
                ss << "and ";
            }
        }
        return ss.str();
    }

    Type *peer_type(std::vector<Type *> types, garbage_collector &gc) {
        auto base_type = types[0];
        for (int i = 1; i < types.size(); i++) {
            if (types[i] != base_type)
                base_type = base_type->peer(types[i], gc);
            if (base_type == nullptr) {
                throw CurdleError{
                        "No Peer Type: cannot find a peer type between " + get_peer_type_list(types),
                        error::ErrorCode::NoPeerType,
                };
            }
        }
        return base_type;
    }

    bool trivial_arithmetic_type(Type *type) {
#define TRIVIAL(T) if (dynamic_cast<T*>(type)) return true
        TRIVIAL(IntegerType);
        TRIVIAL(ComptimeIntegerType);
        TRIVIAL(Float64Type);
        TRIVIAL(ComptimeFloatType);
#undef TRIVIAL
        return false;
    }

    const char *InvalidPeerTypeException::what() const noexcept {
        return message.c_str();
    }
}