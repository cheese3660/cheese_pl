//
// Created by Lexi Allen on 4/1/2023.
//

#ifndef CHEESE_FUNCTIONS_H
#define CHEESE_FUNCTIONS_H


#include "memory/garbage_collection.h"

#include "comptime.h"
#include "runtime.h"
#include "parser/parser.h"

#include <string>
#include <vector>
#include "exception"

namespace cheese::curdle {
    using namespace cheese::memory::garbage_collection;
    struct ComptimeContext;
    struct RuntimeContext;
    struct LocalContext;

    struct FunctionTemplateArgument {
        gcref<Type> type;
        std::string name;
        Comptimeness comptimeness;
    };

    struct FunctionInfo {
        std::vector<FunctionTemplateArgument> arguments;
        int closeness;
        gcref<Type> return_type; // If return type is "any" then it gets defined automatically by the function using peer type resolution
    };

    struct PassedFunctionArgument {
        bool is_type;
        ComptimeValue *value;
        Type *type;
    };

    // This is the most absolute form of a function
    struct ConcreteFunction : managed_object {
        std::string mangled_name; //The mangled name of the function
        Type *returnType;
        std::vector<PassedFunctionArgument> arguments;

        int closeness(const std::vector<PassedFunctionArgument> &args);

        void mark_references() override;

        ~ConcreteFunction() override = default;

        bool is_comptime_only;
        ComptimeValue *returned_value; // If this is void then fun
        ConcreteFunction(std::string path, const std::vector<PassedFunctionArgument> &arguments, Type *returnType,
                         bool comptimeOnly, bool external, bool entry = false);

        void generate_code(ComptimeContext *cctx, RuntimeContext *rctx, bool external,
                           parser::NodePtr body_ptr, bool is_generator, const std::vector<std::string> &rtime_names);
    };

    struct FunctionTemplate : managed_object {
        FunctionTemplate(ComptimeContext *pContext, std::shared_ptr<parser::Node> sharedPtr);

        ComptimeContext *ctx;
        parser::NodePtr ptr;

//        std::vector<Comptimeness> ctime_values();

        std::vector<ConcreteFunction *> concrete_functions;


        void mark_references() override;

        ~FunctionTemplate() override = default;


        FunctionInfo
        get_info_for_arguments(const std::vector<PassedFunctionArgument> &arguments, bool any_zero = false) const;


        // The concrete func
        ConcreteFunction *get(const std::vector<PassedFunctionArgument> &arguments);

        // Gets a function w/o any actual arguments passed to it
        ConcreteFunction *get();
    };

    class IncorrectCallException : std::exception {
    public:
        explicit IncorrectCallException();

    private:
        [[nodiscard]] const char *what() const noexcept override;
    };


    struct FunctionSet : managed_object {

        void mark_references() override;

        ~FunctionSet() override = default;

        std::vector<FunctionTemplate *> templates;

        ConcreteFunction *get(const std::vector<PassedFunctionArgument> &arguments);

        ConcreteFunction *get();
    };


    ConcreteFunction *get_function(RuntimeContext *rctx, ComptimeContext *cctx, LocalContext *lctx,
                                   FunctionSet *function_set, parser::NodeList &arg_nodes);
}
#endif //CHEESE_FUNCTIONS_H