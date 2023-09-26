//
// Created by Lexi Allen on 5/24/2023.
//

#ifndef CHEESE_BUILTIN_H
#define CHEESE_BUILTIN_H

#include <map>
#include <string>
#include "bacteria/BacteriaNode.h"
#include "parser/parser.h"
#include "curdle/runtime.h"
#include "curdle/comptime.h"
#include "bacteria/nodes/expression_nodes.h"
#include "project/GlobalContext.h"

using namespace cheese::project;

namespace cheese::curdle {


    // A builtin isn't a managed object I don't think, but I could be wrong, I am going to need to do some template magic
    // to define builtins though

    struct BadBuiltinCall : std::runtime_error {
        explicit BadBuiltinCall(const std::string &message);

    };

    struct Builtin {
        Builtin() : comptime(false), runtime(false), comptime_function(nullptr), runtime_function(nullptr),
                    get_function(nullptr) {}

        Builtin(gcref<ComptimeValue>
                (*get_function)(Coordinate location, ComptimeContext *cctx, RuntimeContext *rctx)) : comptime(false),
                                                                                                     runtime(false),
                                                                                                     comptime_function(
                                                                                                             nullptr),
                                                                                                     runtime_function(
                                                                                                             nullptr),
                                                                                                     get_function(
                                                                                                             get_function) {

        }

        Builtin(gcref<ComptimeValue>
                (*comptime_function)(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
                                     RuntimeContext *rctx))
                : comptime(true), runtime(false), comptime_function(comptime_function), runtime_function(nullptr),
                  get_function(nullptr) {
        }

        Builtin(bacteria::BacteriaPtr
                (*runtime_function)(Coordinate location, LocalContext *ctx, std::vector<parser::Node *> arguments))
                : comptime(false), runtime(true), runtime_function(runtime_function), comptime_function(nullptr),
                  get_function(nullptr) {}

        Builtin(gcref<ComptimeValue>
                (*comptime_function)(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
                                     RuntimeContext *rctx),
                bacteria::BacteriaPtr
                (*runtime_function)(Coordinate location, LocalContext *ctx, std::vector<parser::Node *> arguments))
                : comptime(true), runtime(true), comptime_function(comptime_function),
                  runtime_function(runtime_function) {}
        // Now we must define the entire builtin structure that will be used

        bool comptime;
        bool runtime;

        gcref<ComptimeValue>
        exec(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
             RuntimeContext *rctx = nullptr) {
            if (!comptime) {
                throw BadBuiltinCall("Bad Builtin Call: Attempting to run a non-compile time builtin");
            }
            return comptime_function(location, std::move(arguments), cctx, rctx);
        }

        std::optional<gcref<ComptimeValue>>
        try_exec(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
                 RuntimeContext *rctx = nullptr) {
            try {
                return exec(location, std::move(arguments), cctx, rctx);
            } catch (NotComptimeError &) {
                return {};
            } catch (BadBuiltinCall &) {
                return {};
            }
        }

        bacteria::BacteriaPtr translate(Coordinate location, LocalContext *ctx, std::vector<parser::Node *> arguments) {
            if (!runtime) {
                ctx->runtime->comptime->globalContext->raise(
                        "Bad Builtin Call: Attempting to translate a non-runtime builtin at runtime", location,
                        error::ErrorCode::BadBuiltinCall);
                return std::make_unique<bacteria::nodes::Nop>(location);
            }
            return runtime_function(location, ctx, std::move(arguments));
        }

        gcref<ComptimeValue>
        get(Coordinate location, ComptimeContext *cctx, RuntimeContext *rctx) {
            if (comptime || runtime) {
                throw BadBuiltinCall("Bad Builtin Call: Trying to get the value of a functional builtin");
            } else {
                return get_function(location, cctx, rctx);
            }
        }

    private:
        gcref<ComptimeValue>
        (*comptime_function)(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
                             RuntimeContext *rctx);

        bacteria::BacteriaPtr
        (*runtime_function)(Coordinate location, LocalContext *ctx, std::vector<parser::Node *> arguments);

        gcref<ComptimeValue>
        (*get_function)(Coordinate location, ComptimeContext *cctx, RuntimeContext *rctx);

        // We might add a third value for "getting a cached value" when its a single value builtin
    };


    extern std::map<std::string, Builtin> builtins;

    struct BuiltinWrapper {
        BuiltinWrapper(std::string name, Builtin builtin) {
            builtins[name] = builtin;
        }
    };
}

#define BUILTIN_NAME_MERGE(a, b) a##b
#define BUILTIN_NAME(a) BUILTIN_NAME_MERGE(builtin_,a)
#define BUILTIN(name, value) static BuiltinWrapper BUILTIN_NAME(__LINE__){name, Builtin{value}};
#define BUILTIN2(name, funct, funct2) static BuiltinWrapper BUILTIN_NAME(__LINE__){name, Builtin{funct,funct2}};

#endif //CHEESE_BUILTIN_H
