//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_GLOBALCONTEXT_H
#define CHEESE_GLOBALCONTEXT_H

#include "Project.h"
#include "Structure.h"

namespace cheese::curdle {
    struct GlobalContext {
        const Project &project;
        std::shared_ptr<Structure> root_structure;

    };
}
#endif //CHEESE_GLOBALCONTEXT_H
