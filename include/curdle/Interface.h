//
// Created by Lexi Allen on 4/1/2023.
//

#ifndef CHEESE_INTERFACE_H
#define CHEESE_INTERFACE_H

#include "curdle/Type.h"

namespace cheese::curdle {
    struct Interface : Type {
        bacteria::TypePtr get_bacteria_type() override;

        void mark_type_references() override;

        ~Interface() override = default;
    };
}

#endif //CHEESE_INTERFACE_H
