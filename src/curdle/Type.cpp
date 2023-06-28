//
// Created by Lexi Allen on 4/1/2023.
//

#include "curdle/Type.h"
#include "curdle/comptime.h"
#include "error.h"
#include "curdle/curdle.h"
#include <typeinfo>
#include "curdle/GlobalContext.h"

namespace cheese::curdle {
#define PEER_TYPE_CATCH_ANY() if (dynamic_cast<AnyType*>(other)) return gcref{gctx->gc,this}
#define NO_PEER gcref<Type>{gctx->gc,nullptr}
#define REF(X) gcref{gctx->gc,X}
#define NO_BACTERIA_TYPE(name) throw cheese::curdle::CurdleError{"No bacteria type for " # name, error::ErrorCode::NoBacteriaType};

    bacteria::TypePtr TypeType::get_bacteria_type() {
        // This should actually throw an error if attempting to convert this to a runtime (bacteria) type
        return {};
    }

    void TypeType::mark_type_references() {

    }


    TypeType *TypeType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: type")) {
            auto ref = gctx->gc.gcnew<TypeType>();
            gctx->cached_objects["type: type"] = ref;
            return ref;
        }
        return dynamic_cast<TypeType *>(gctx->cached_objects["type: type"]);
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

    gcref<Type> TypeType::peer(Type *other, GlobalContext *gctx) {
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
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

    IntegerType *IntegerType::get(GlobalContext *gctx, bool sign, std::uint16_t size) {
        std::string name = std::string("type: ") + (sign ? "i" : "u") + std::to_string(size);
        if (!gctx->cached_objects.contains(name)) {
            auto ref = gctx->gc.gcnew<IntegerType>(sign, size);
            gctx->cached_objects[name] = ref;
            return ref;
        }
        return dynamic_cast<IntegerType *>(gctx->cached_objects[name]);
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

    gcref<Type> IntegerType::peer(Type *other, GlobalContext *gctx) {
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
        if (dynamic_cast<Float64Type *>(other)) {
            return REF(other);
        }
        if (dynamic_cast<ComptimeFloatType *>(other)) {
            return REF(other);
        }
        return NO_PEER;
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

    gcref<Type> ReferenceType::peer(Type *other, GlobalContext *gctx) {
        if (compare(other) == 0) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }

    bacteria::TypePtr VoidType::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Void);
    }

    void VoidType::mark_type_references() {

    }

    static VoidType *vt_instance;

    VoidType *VoidType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: void")) {
            auto ref = gctx->gc.gcnew<VoidType>();
            gctx->cached_objects["type: void"] = ref;
            return ref;
        }
        return dynamic_cast<VoidType *>(gctx->cached_objects["type: void"]);
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

    gcref<Type> VoidType::peer(Type *other, GlobalContext *gctx) {
        PEER_TYPE_CATCH_ANY();
    }

    bacteria::TypePtr AnyType::get_bacteria_type() {
        NO_BACTERIA_TYPE(any);
    }

    void AnyType::mark_type_references() {

    }


    AnyType *AnyType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: any")) {
            auto ref = gctx->gc.gcnew<AnyType>();
            gctx->cached_objects["type: any"] = ref;
            return ref;
        }
        return dynamic_cast<AnyType *>(gctx->cached_objects["type: any"]);
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

    gcref<Type> AnyType::peer(Type *other, GlobalContext *gctx) {
        return REF(other);
    }

    bacteria::TypePtr FunctionTemplateType::get_bacteria_type() {
        return {};
    }

    void FunctionTemplateType::mark_type_references() {
    }


    FunctionTemplateType *FunctionTemplateType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: function_template")) {
            auto ref = gctx->gc.gcnew<FunctionTemplateType>();
            gctx->cached_objects["type: function_template"] = ref;
            return ref;
        }
        return dynamic_cast<FunctionTemplateType *>(gctx->cached_objects["type: function_template"]);
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

    gcref<Type> FunctionTemplateType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return REF(other);
    }

    bacteria::TypePtr ComptimeIntegerType::get_bacteria_type() {
        NO_BACTERIA_TYPE(comptime_int);
    }

    static ComptimeIntegerType *cit_instance;

    void ComptimeIntegerType::mark_type_references() {

    }

    ComptimeIntegerType *ComptimeIntegerType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: comptime_int")) {
            auto ref = gctx->gc.gcnew<ComptimeIntegerType>();
            gctx->cached_objects["type: comptime_int"] = ref;
            return ref;
        }
        return dynamic_cast<ComptimeIntegerType *>(gctx->cached_objects["type: comptime_int"]);
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

    gcref<Type> ComptimeIntegerType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        if (auto other_i = dynamic_cast<IntegerType *>(other); other_i) {
            return REF(other_i);
        } else if (auto other_f = dynamic_cast<Float64Type *>(other); other_f) {
            return REF(other_f);
        } else {
            return NO_PEER;
        }
    }

    bacteria::TypePtr BooleanType::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::UnsignedInteger, 1);
    }

    void BooleanType::mark_type_references() {

    }

    BooleanType *BooleanType::get(GlobalContext *gctx) {
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

    gcref<Type> BooleanType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }


    bacteria::TypePtr BuiltinReferenceType::get_bacteria_type() {
        NO_BACTERIA_TYPE(builtin_reference);
    }

    void BuiltinReferenceType::mark_type_references() {

    }

    BuiltinReferenceType *BuiltinReferenceType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: builtin_reference")) {
            auto ref = gctx->gc.gcnew<BuiltinReferenceType>();
            gctx->cached_objects["type: builtin_reference"] = ref;
            return ref;
        }
        return dynamic_cast<BuiltinReferenceType *>(gctx->cached_objects["type: builtin_reference"]);
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

    gcref<Type> BuiltinReferenceType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }

    NoReturnType *NoReturnType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: noreturn")) {
            auto ref = gctx->gc.gcnew<NoReturnType>();
            gctx->cached_objects["type: noreturn"] = ref;
            return ref;
        }
        return dynamic_cast<NoReturnType *>(gctx->cached_objects["type: noreturn"]);
    }

    bacteria::TypePtr NoReturnType::get_bacteria_type() {
        NO_BACTERIA_TYPE(noreturn);
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

    gcref<Type> NoReturnType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return REF(other);
    }

    bacteria::TypePtr Float64Type::get_bacteria_type() {
        return std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Float64);
    }

    void Float64Type::mark_type_references() {

    }

    Float64Type *Float64Type::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: f64")) {
            auto ref = gctx->gc.gcnew<Float64Type>();
            gctx->cached_objects["type: f64"] = ref;
            return ref;
        }
        return dynamic_cast<Float64Type *>(gctx->cached_objects["type: f64"]);
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

    gcref<Type> Float64Type::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        if (dynamic_cast<IntegerType *>(other) || dynamic_cast<ComptimeIntegerType *>(other) ||
            dynamic_cast<ComptimeFloatType *>(other)) {
            return REF(this);
        }
        return NO_PEER;
    }

    bacteria::TypePtr ComptimeFloatType::get_bacteria_type() {
        NO_BACTERIA_TYPE(comptime_float);
    }


    void ComptimeFloatType::mark_type_references() {

    }

    ComptimeFloatType *ComptimeFloatType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: comptime_float")) {
            auto ref = gctx->gc.gcnew<ComptimeFloatType>();
            gctx->cached_objects["type: comptime_float"] = ref;
            return ref;
        }
        return dynamic_cast<ComptimeFloatType *>(gctx->cached_objects["type: comptime_float"]);
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


    gcref<Type> ComptimeFloatType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        if (dynamic_cast<ComptimeIntegerType *>(other)) {
            return REF(this);
        }
        if (dynamic_cast<Float64Type *>(other) || dynamic_cast<IntegerType *>(other)) {
            return REF(other);
        }
        return NO_PEER;
    }


    bacteria::TypePtr ErrorType::get_bacteria_type() {
        NO_BACTERIA_TYPE(error_type);
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
    gcref<Type> ErrorType::peer(Type *other, GlobalContext *gctx) {
        if (other == this) return REF(this);
        PEER_TYPE_CATCH_ANY();
        return NO_PEER;
    }

    std::string ErrorType::to_string() {
        return "ERROR!";
    }


    ErrorType *ErrorType::get(GlobalContext *gctx) {
        if (!gctx->cached_objects.contains("type: error")) {
            auto ref = gctx->gc.gcnew<ErrorType>();
            gctx->cached_objects["type: error"] = ref;
            return ref;
        }
        return dynamic_cast<ErrorType *>(gctx->cached_objects["type: error"]);
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

    gcref<Type> peer_type(std::vector<Type *> types, GlobalContext *gctx) {
        if (types.empty()) {
            throw CurdleError{"No Peer Type: cannot find a peer type w/o any types to find a peer between",
                              error::ErrorCode::NoPeerType};
        }
        auto base_type = gcref{gctx->gc, types[0]};
        for (int i = 1; i < types.size(); i++) {
            if (types[i] != base_type)
                base_type = base_type->peer(types[i], gctx);
            if (base_type.value == nullptr) {
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