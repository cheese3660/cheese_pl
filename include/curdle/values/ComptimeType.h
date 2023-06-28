//
// Created by Lexi Allen on 6/28/2023.
//

#ifndef CHEESE_COMPTIMETYPE_H
#define CHEESE_COMPTIMETYPE_H

#include "curdle/comptime.h"

namespace cheese::curdle {
    struct ComptimeType : ComptimeValue {
        ComptimeType(GlobalContext *gctx, Type *pType);

        void mark_value() override;

        Type *typeValue;

        ~ComptimeType() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;
    };
}

#endif //CHEESE_COMPTIMETYPE_H
