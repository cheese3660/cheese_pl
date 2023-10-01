//
// Created by Lexi Allen on 9/30/2023.
//

#ifndef CHEESE_COMPTIMEENUMLITERAL_H
#define CHEESE_COMPTIMEENUMLITERAL_H

#include "curdle/comptime.h"

namespace cheese::curdle {
    struct ComptimeEnumLiteral : ComptimeValue {
        explicit ComptimeEnumLiteral(std::string value, Type *type);

        explicit ComptimeEnumLiteral(std::string value, std::vector<ComptimeValue *> tuple_fields, Type *type);

        explicit ComptimeEnumLiteral(std::string value,
                                     std::unordered_map<std::string, ComptimeValue *> structured_fields, Type *type);

        void mark_value() override;

        std::string value;
        std::vector<ComptimeValue *> tuple_fields;
        std::unordered_map<std::string, ComptimeValue *> structured_fields;

        ~ComptimeEnumLiteral() override = default;

        bool is_same_as(ComptimeValue *other) override;

        std::string to_string() override;

        gcref<ComptimeValue> cast(Type *target_type, garbage_collector &garbageCollector) override;

        gcref<ComptimeValue> op_tuple_call(GlobalContext *gctx, std::vector<ComptimeValue *> values) override;

        gcref<ComptimeValue>
        op_object_call(GlobalContext *gctx, std::unordered_map<std::string, ComptimeValue *> values) override;


    };
}

#endif //CHEESE_COMPTIMEENUMLITERAL_H
