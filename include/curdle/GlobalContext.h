//
// Created by Lexi Allen on 3/20/2023.
//

#ifndef CHEESE_GLOBALCONTEXT_H
#define CHEESE_GLOBALCONTEXT_H

#include "Project.h"
#include "Structure.h"
#include "memory/garbage_collection.h"


namespace cheese::curdle {
    struct GlobalContext {
        const Project &project;
        Structure *root_structure;
        memory::garbage_collection::garbage_collector gc;
    };
}
#endif //CHEESE_GLOBALCONTEXT_H
