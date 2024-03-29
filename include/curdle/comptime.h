//
// Created by Lexi Allen on 3/20/2023.
//


#ifndef CHEESE_COMPTIME_H
#define CHEESE_COMPTIME_H

#include <utility>

#include <utility>

#include "memory/garbage_collection.h"
#include "curdle/Type.h"
#include "math/BigInteger.h"
#include "curdle/types/Structure.h"
#include "parser/nodes/other_nodes.h"
#include "parser/nodes/single_member_nodes.h"

#include "parser/nodes/terminal_nodes.h"
#include "curdle/runtime.h"
#include "project/GlobalContext.h"
#include "error.h"

using namespace cheese::project;
//#include "builtin.h"

namespace cheese::project {
    struct GlobalContext;
}

namespace cheese::curdle {
    struct FunctionSet;
    struct Builtin;
    using namespace memory::garbage_collection;
    namespace fs = std::filesystem;
    // Have to forward declare these for some reason?
    struct Structure;

    struct BadComptimeCastError : std::runtime_error {
        BadComptimeCastError(const std::string &message);
    };

    struct InvalidCastError : std::runtime_error {
        InvalidCastError(const std::string &message) : std::runtime_error(message) {};
    };

    struct NotComptimeError : std::runtime_error {
        NotComptimeError(const std::string &message) : std::runtime_error(message) {};
    };


    struct ComptimeValue : managed_object {

        virtual void mark_value() = 0;

        virtual bool is_same_as(ComptimeValue *other) = 0;

        virtual gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) = 0;

        virtual std::string to_string() = 0;

        void mark_references() override;


        memory::garbage_collection::gcref<Type>
        binary_peer_lhs(Type *other_type, bool &cast_self, cheese::project::GlobalContext *gctx);


        template<typename T>
        gcref<T> binary_peer_rhs(ComptimeValue *other, Type *peer, cheese::project::GlobalContext *gctx) {
            gcref<ComptimeValue> rhs = {gctx->gc, other};
            auto rhs_compare = peer->compare(other->type);
            if (rhs_compare == -1)
                binary_peer_error(
                        "Bad Compile Time Cast: cannot cast a value of type " + other->type->to_string() +
                        " to a value of type " + peer->to_string(), error::ErrorCode::BadComptimeCast);
            if (rhs_compare != 0) rhs = other->cast(peer, gctx->gc); else rhs = {gctx->gc, other};
            return {gctx->gc, dynamic_cast<T *>(rhs.get())};
        }

        // A comptime value can have many types
        Type *type;

        // Now we implement the actual interfaces for everything, addition, subtraction, any operation that can be overloaded at compile time
        // This is before we implement mixins, once we add those we might need to rewrite some stuff

        virtual gcref<ComptimeValue> op_tuple_call(GlobalContext *gctx, std::vector<ComptimeValue *> values);

        virtual gcref<ComptimeValue> op_array_call(GlobalContext *gctx, std::vector<ComptimeValue *> values);

        virtual gcref<ComptimeValue>
        op_object_call(GlobalContext *gctx, std::unordered_map<std::string, ComptimeValue *> values);

        virtual gcref<ComptimeValue> op_unary_plus(GlobalContext *gctx);

        virtual gcref<ComptimeValue> op_unary_minus(GlobalContext *gctx);

        virtual gcref<ComptimeValue> op_not(GlobalContext *gctx);

        virtual gcref<ComptimeValue> op_multiply(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_divide(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_remainder(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_add(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_subtract(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_left_shift(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_right_shift(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_lesser_than(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_greater_than(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_lesser_than_equal(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_greater_than_equal(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_equal(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_not_equal(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_and(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_xor(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_or(GlobalContext *gctx, ComptimeValue *other);

        virtual gcref<ComptimeValue> op_combine(GlobalContext *gctx, ComptimeValue *other);

    private:
        static void binary_peer_error(std::string msg, error::ErrorCode errorCode);
    };


    struct ComptimeVariable : managed_object {
        ComptimeVariable(Type *declaringType, ComptimeValue *value) : declaringType(declaringType), value(value) {}

        void mark_references() override;

        Type *declaringType; //This can be `any` and such
        ComptimeValue *value;

        ~ComptimeVariable() override = default;
    };


    struct ComptimeContext : managed_object {


        void mark_references() override;

        Structure *currentStructure;
        ComptimeContext *parent;
        cheese::project::GlobalContext *globalContext;
        std::unordered_map<std::string, ComptimeVariable *> comptimeVariables;
        std::vector<std::string> structure_name_stack;
        size_t next_offset_for_structure_name = 0;

        fs::path path;
        fs::path project_dir; // This is $newProjectDir in the reference

        ComptimeContext(cheese::project::GlobalContext *gc, fs::path p, fs::path d) : globalContext(gc),
                                                                                      parent(nullptr),
                                                                                      currentStructure(nullptr),
                                                                                      path(std::move(p)),
                                                                                      project_dir(std::move(d)) {

        }

        ComptimeContext(ComptimeContext *p) : currentStructure(p->currentStructure), parent(p),
                                              globalContext(p->globalContext), path(p->path),
                                              project_dir(p->project_dir) {
        }


        ComptimeContext(ComptimeContext *p, Structure *c) : currentStructure(c), parent(p),
                                                            globalContext(p->globalContext), path(p->path),
                                                            project_dir(p->project_dir) {

        }


        std::optional<gcref<ComptimeValue>> try_exec(parser::Node *node, RuntimeContext *rtime = nullptr);

        std::optional<gcref<ComptimeValue>> get(const std::string &name);

        gcref<ComptimeValue> exec(parser::Node *node, RuntimeContext *rtime = nullptr);

        gcref<ComptimeValue> exec(parser::NodePtr node, RuntimeContext *rtime = nullptr);

        gcref<ComptimeValue> exec(parser::nodes::Comptime *ctime, RuntimeContext *rtime = nullptr);

        ~ComptimeContext() override = default;

        gcref<ComptimeValue> exec_tuple_call(parser::nodes::TupleCall *node, RuntimeContext *rtime);

//        std::optional<gcref<ComptimeValue>> try_exec_tuple_call(parser::nodes::TupleCall *node, RuntimeContext *rtime);

        std::string get_structure_name();

        void push_structure_name(std::string);

        void pop_structure_name();

        gcref<ComptimeValue> exec_object_call(parser::nodes::ObjectCall *call, RuntimeContext *rtime);
    };
}
#endif //CHEESE_COMPTIME_H
