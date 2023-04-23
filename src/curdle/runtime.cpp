//
// Created by Lexi Allen on 4/6/2023.
//
#include "curdle/runtime.h"


namespace cheese::curdle {

    void RuntimeContext::mark_references() {
        for (auto &var: variables) {
            var.second.type->mark();
        }
        comptime->mark();
        if (parent) parent->mark();
        if (structure) structure->mark();

    }

    std::optional<RuntimeVariableInfo> RuntimeContext::get(const std::string &name) {
        // This thing is only runtime, comptime is done in the stage before;
        if (variables.contains(name)) {
            return variables[name];
        }
        if (parent) {
            auto v = parent->get(name);
            if (v.has_value()) {
                return v.value();
            }
        }
        if (structure) {
            if (structure->top_level_variables.contains(name)) {
                auto &v = structure->top_level_variables[name];
                return RuntimeVariableInfo{v.constant, v.type};
            }
        }
        return {};
    }
}