//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_TYPE_H
#define CHEESE_TYPE_H

#include "bacteria/BacteriaType.h"
#include "Mixin.h"
#include "memory/garbage_collection.h"

namespace cheese::curdle {
    enum class Comptimeness {
        Comptime,
        ArgumentDepending, //For "any" types
        Runtime
    };

    struct Type : memory::garbage_collection::managed_object {
        // This function is used for converting a type into bacteria.
        bacteria::TypePtr get_cached_type();

        virtual bacteria::TypePtr get_bacteria_type() = 0;

        virtual void mark_type_references() = 0;

        virtual Comptimeness get_comptimeness() = 0;

        // This should be done in a way that the other type is *always* concrete and not an "any"
        virtual std::int32_t compare(Type *other) = 0;

        virtual Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) = 0;

        virtual std::string to_string() = 0;

        // This is where we add mixins and such

        std::vector<Mixin *> mixins;
        // So we don't have to do funky stuff :3
        bacteria::TypePtr cached_type;

        void mark_references() final;


        ~Type() override = default;
    };

    struct TypeType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~TypeType() override = default;

        static TypeType *get(memory::garbage_collection::garbage_collector &gc);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;

    private:

        TypeType() = default;
    };

    struct ComptimeIntegerType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~ComptimeIntegerType() override = default;

        static ComptimeIntegerType *get(memory::garbage_collection::garbage_collector &gc);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;

    private:

        ComptimeIntegerType() = default;
    };


    struct IntegerType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bool sign{false};
        std::uint16_t size;

        static IntegerType *get(memory::garbage_collection::garbage_collector &gc, bool sign, std::uint16_t size);

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~IntegerType() override = default;

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;

    private:

        IntegerType(memory::garbage_collection::garbage_collector &gc, bool sn, std::uint16_t sz);
    };


    struct ReferenceType : Type {
        ReferenceType(Type *child, bool constant) : child(child), constant(constant) {}

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~ReferenceType() override = default;

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;

        Type *child;
        bool constant; //Whether this is a constant reference type

    };

    struct VoidType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~VoidType() override = default;

        static VoidType *get(memory::garbage_collection::garbage_collector &gc);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;

    private:

        VoidType() = default;
    };

    struct AnyType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~AnyType() override = default;

        static AnyType *get(memory::garbage_collection::garbage_collector &gc);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;

    private:

        AnyType() = default;
    };

    struct BooleanType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~BooleanType() override = default;

        static BooleanType *get(memory::garbage_collection::garbage_collector &gc);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;

    private:

        BooleanType() = default;
    };

    struct FunctionTemplateType : Type {
        friend class cheese::memory::garbage_collection::garbage_collector;

        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~FunctionTemplateType() override = default;

        static FunctionTemplateType *get(memory::garbage_collection::garbage_collector &gc);

        Comptimeness get_comptimeness() override;

        int32_t compare(Type *other) override;

        std::string to_string() override;

        Type *peer(Type *other, memory::garbage_collection::garbage_collector &gc) override;

    private:
        FunctionTemplateType() = default;
    };

    class InvalidPeerTypeException : std::exception {
    public:
        ~InvalidPeerTypeException() noexcept override = default;

        const char *what() const override;

    };

    Type *peer_type(std::vector<Type *> types, memory::garbage_collection::garbage_collector &gc);

    bool trivial_arithmetic_type(Type *type);
}
#endif //CHEESE_TYPE_H
