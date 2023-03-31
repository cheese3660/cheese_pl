//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_WEAKTYPE_H
#define CHEESE_WEAKTYPE_H

#include <memory>
#include "Type.h"

namespace cheese::curdle {
    // Used to avoid circular references in a structure
    struct WeakPointer : Type {
        std::weak_ptr<Type> reference;
        bool constant;
    };
}


#endif //CHEESE_WEAKTYPE_H
