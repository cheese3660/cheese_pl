//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_TYPE_H
#define CHEESE_TYPE_H

#include "bacteria/BacteriaType.h"
#include "Mixin.h"
#include "memory/garbage_collection.h"

namespace cheese::curdle {
    struct Type : memory::garbage_collection::managed_object {
        // This function is used for converting a type into bacteria.
        virtual bacteria::TypePtr get_bacteria_type() = 0;

        // This is where we add mixins and such
        std::vector<std::unique_ptr<Mixin>> mixins;
    };
}
#endif //CHEESE_TYPE_H
