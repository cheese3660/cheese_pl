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


namespace cheese::curdle {

    using namespace memory::garbage_collection;
    namespace fs = std::filesystem;
    // Have to forward declare these for some reason?
    struct GlobalContext;
    struct Structure;

    struct ComptimeValue : managed_object {

        virtual void mark_value() = 0;

        void mark_references() override;

        // A comptime value can have many types
        Type *type;
    };


    // An integer of any integral type
    struct ComptimeInteger : ComptimeValue {
        void mark_value() override;

        ~ComptimeInteger() override = default;

        math::BigInteger value;
    };

    struct ComptimeFloat : ComptimeValue {
        void mark_value() override;

        float value;

        ~ComptimeFloat() override = default;
    };

    struct ComptimeDouble : ComptimeValue {
        void mark_value() override;

        double value;

        ~ComptimeDouble() override = default;
    };

    struct ComptimeString : ComptimeValue {
        void mark_value() override;

        std::string value;

        ~ComptimeString() override = default;
    };

    struct ComptimeVoid : ComptimeValue {
        void mark_value() override;

        ~ComptimeVoid() override = default;
    };

    struct ComptimeComplex32 : ComptimeValue {
        void mark_value() override;

        float a;
        float b;

        ~ComptimeComplex32() override = default;
    };

    struct ComptimeComplex64 : ComptimeValue {
        void mark_value() override;

        double a;
        double b;

        ~ComptimeComplex64() override = default;
    };

    struct ComptimeType : ComptimeValue {
        ComptimeType(garbage_collector &gc, Type *pType);

        void mark_value() override;

        Type *typeValue;

        ~ComptimeType() override = default;
    };

    //Array/Tuple/Slice
    struct ComptimeArray : ComptimeValue {
        void mark_value() override;

        std::vector<ComptimeValue *> values;

        ~ComptimeArray() override = default;
    };

    struct ComptimeObject : ComptimeValue {
        void mark_value() override;

        std::map<std::string, ComptimeValue *> fields;

        ~ComptimeObject() override = default;
    };

    struct ComptimeVariable : managed_object {
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


        gcref<ComptimeValue> exec(parser::NodePtr node);

        gcref<ComptimeValue> exec(parser::nodes::Comptime *ctime);

        ~ComptimeContext() override = default;
    };
}
#endif //CHEESE_COMPTIME_H
