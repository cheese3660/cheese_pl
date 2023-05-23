//
// Created by Lexi Allen on 4/3/2023.
//

#ifndef CHEESE_RUNTIME_H
#define CHEESE_RUNTIME_H

#include "memory/garbage_collection.h"
#include "curdle/comptime.h"
#include "curdle/variables.h"
#include "bacteria/BacteriaReciever.h"
#include <string>
#include <map>

namespace cheese::curdle {
    using namespace memory::garbage_collection;
    struct ComptimeContext;
    struct Structure;
    struct RuntimeVariableInfo;
    struct Type;
    struct RuntimeContext;

    // This just contains a lot of stuff that is useful
    struct LocalContext : managed_object {
        RuntimeContext *runtime;
        Type *expected_type;

        std::pair<Type *, Type *> get_binary_type(parser::Node *lhs, parser::Node *rhs);

        Type *get_type(parser::Node *node);

        void mark_references() override;

        ~LocalContext() override = default;

        LocalContext(RuntimeContext *runtime, Type *expected_type = nullptr) : runtime(runtime),
                                                                               expected_type(expected_type) {}

        LocalContext(LocalContext *lctx, Type *expected_type = nullptr) : runtime(lctx->runtime),
                                                                          expected_type(expected_type ? expected_type
                                                                                                      : lctx->expected_type) {}
    };

    struct RuntimeContext : managed_object {

        RuntimeContext(RuntimeContext *p, ComptimeContext *c, Structure *s) : parent(p), comptime(c), structure(s),
                                                                              functionReturnType(nullptr) {}

        RuntimeContext(ComptimeContext *c, Structure *s) : parent(nullptr), comptime(c), structure(s),
                                                           functionReturnType(nullptr) {}

        bacteria::BacteriaReciever *local_reciever{nullptr};
        RuntimeContext *parent;
        ComptimeContext *comptime;
        Structure *structure;
        Type *functionReturnType{nullptr}; // This is the return type of a function


        std::map<std::string, RuntimeVariableInfo> variables{};

        std::optional<RuntimeVariableInfo> get(const std::string &name);

        Type *get_type(parser::Node *node);

        void mark_references() override;
    };
};
#endif //CHEESE_RUNTIME_H
