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
#include "curdle/Structure.h"
#include "parser/nodes/other_nodes.h"
#include "parser/nodes/single_member_nodes.h"

#include "parser/nodes/terminal_nodes.h"
#include "curdle/runtime.h"


namespace cheese::curdle {
    struct FunctionSet;
    using namespace memory::garbage_collection;
    namespace fs = std::filesystem;
    // Have to forward declare these for some reason?
    struct GlobalContext;
    struct Structure;

    struct ComptimeValue : managed_object {

        virtual void mark_value() = 0;

        virtual bool is_same_as(ComptimeValue *other) = 0;

        virtual std::string to_string() = 0;

        void mark_references() override;

        // A comptime value can have many types
        Type *type;
    };


    // An integer of any integral type
    struct ComptimeInteger : ComptimeValue {
        explicit ComptimeInteger(const math::BigInteger &value, Type *ty);

        void mark_value() override;

        ~ComptimeInteger() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        math::BigInteger value;
    };

    struct ComptimeFloat : ComptimeValue {
        void mark_value() override;

        double value;

        ~ComptimeFloat() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;
    };

    struct ComptimeString : ComptimeValue {
        void mark_value() override;

        std::string value;

        ~ComptimeString() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;
    };

    struct ComptimeVoid : ComptimeValue {
        void mark_value() override;

        ~ComptimeVoid() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;
    };

    struct ComptimeComplex : ComptimeValue {
        void mark_value() override;

        double a;
        double b;

        ~ComptimeComplex() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;
    };

    struct ComptimeType : ComptimeValue {
        ComptimeType(garbage_collector &gc, Type *pType);

        void mark_value() override;

        Type *typeValue;

        ~ComptimeType() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;
    };

    //Array/Tuple/Slice
    struct ComptimeArray : ComptimeValue {
        void mark_value() override;

        std::vector<ComptimeValue *> values;

        ~ComptimeArray() override = default;

        bool is_same_as(ComptimeValue *other) override;
    };

    struct ComptimeObject : ComptimeValue {
        void mark_value() override;

        std::map<std::string, ComptimeValue *> fields;

        ~ComptimeObject() override = default;

        bool is_same_as(ComptimeValue *other) override;
    };

    struct ComptimeFunctionSet : ComptimeValue {
        FunctionSet *set;

        explicit ComptimeFunctionSet(FunctionSet *set);

        void mark_value() override;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        ~ComptimeFunctionSet() override = default;

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
        GlobalContext *globalContext;
        std::map<std::string, ComptimeVariable *> comptimeVariables;
        fs::path path;
        fs::path project_dir; // This is $newProjectDir in the reference

        ComptimeContext(GlobalContext *gc, fs::path p, fs::path d) : globalContext(gc), parent(nullptr),
                                                                     currentStructure(nullptr), path(std::move(p)),
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

    };
}
#endif //CHEESE_COMPTIME_H
