//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_MIXIN_H
#define CHEESE_MIXIN_H

#include "memory/garbage_collection.h"

namespace cheese::curdle {
    struct Mixin : cheese::memory::garbage_collection::managed_object {
        void mark_references() override;
        
        ~Mixin() override = default;
    };
}

#endif //CHEESE_MIXIN_H
