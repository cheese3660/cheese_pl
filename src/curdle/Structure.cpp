//
// Created by Lexi Allen on 3/30/2023.
//

#include "curdle/Structure.h"

namespace cheese::curdle {

    bacteria::TypePtr Structure::get_bacteria_type() {
        return cheese::bacteria::TypePtr();
    }

    void Structure::mark_references() {
        for (auto &field: fields) {
            field.type->mark();
        }
        for (auto &var: top_level_variables) {
            var.second.type->mark();
        }
    }

    Structure::~Structure() {

    }
}
