//
// Created by Lexi Allen on 10/1/2023.
//

#ifndef CHEESE_IMPORTEDFUNCTION_H
#define CHEESE_IMPORTEDFUNCTION_H

#include "curdle/comptime.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {
    struct ImportedFunction : ComptimeValue {
        ImportedFunction(std::string function, Type *prototype);

        std::string function;

        void mark_value() override;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        ~ImportedFunction() override = default;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

    };
}
#endif //CHEESE_IMPORTEDFUNCTION_H
