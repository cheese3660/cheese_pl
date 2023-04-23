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
        virtual bacteria::TypePtr get_bacteria_type() = 0;

        virtual void mark_type_references() = 0;

        virtual Comptimeness get_comptimeness() = 0;

        // This should be done in a way that the other type is *always* concrete and not an "any"
        virtual std::int32_t compare(Type *other) = 0;

        // This is where we add mixins and such

        std::vector<Mixin *> mixins;

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

    private:

        TypeType() = default;
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

    private:

        AnyType() = default;
    };
}
#endif //CHEESE_TYPE_H
