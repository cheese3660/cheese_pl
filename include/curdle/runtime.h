//
// Created by Lexi Allen on 4/3/2023.
//

#ifndef CHEESE_RUNTIME_H
#define CHEESE_RUNTIME_H

#include "memory/garbage_collection.h"
#include "curdle/comptime.h"
#include "curdle/variables.h"
#include <string>
#include <map>

namespace cheese::curdle {
    using namespace memory::garbage_collection;

    struct RuntimeContext : managed_object {

        RuntimeContext(RuntimeContext *p, ComptimeContext *c, Structure *s) : parent(p), comptime(c), structure(s) {}

        RuntimeContext(ComptimeContext *c, Structure *s) : parent(nullptr), comptime(c), structure(s) {}

        RuntimeContext *parent;
        ComptimeContext *comptime;
        Structure *structure;


        std::map<std::string, RuntimeVariableInfo> variables{};

        std::optional<RuntimeVariableInfo> get(const std::string &name);

        void mark_references() override;
    };
};
#endif //CHEESE_RUNTIME_H
