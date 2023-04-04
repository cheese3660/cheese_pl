//
// Created by Lexi Allen on 4/1/2023.
//

#ifndef CHEESE_FUNCTIONS_H
#define CHEESE_FUNCTIONS_H


#include "memory/garbage_collection.h"

#include "comptime.h"
#include "parser/parser.h"

#include <string>
#include <vector>

namespace cheese::curdle {
    using namespace cheese::memory::garbage_collection;
    struct ComptimeContext;

    struct FunctionTemplateArgument {
        Type *type;
        std::string name;
        Comptimeness comptimeness;
    };

    struct ConcreteFunctionArgument {
        Type *type;
        std::string name;
        bool comptime;
    };

    // This is the most absolute form of a function
    struct ConcreteFunction : managed_object {
        std::string mangled_name; //The mangled name of the function
    };

    struct FunctionTemplate : managed_object {
        FunctionTemplate(ComptimeContext *pContext, std::shared_ptr<parser::Node> sharedPtr);

        bool defined{false};


        ComptimeContext *ctx;
        parser::NodePtr ptr;

        std::vector<FunctionTemplateArgument> arguments;

        Type *return_type;

        std::vector<Comptimeness> ctime_values();

        void define();

        void mark_references() override;

        ~FunctionTemplate() override = default;
    };

    struct FunctionSet : managed_object {

        void mark_references() override;

        ~FunctionSet() override = default;

        std::vector<FunctionTemplate *> templates;
    };
}
#endif //CHEESE_FUNCTIONS_H