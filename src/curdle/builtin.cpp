//
// Created by Lexi Allen on 5/24/2023.
//

#include "curdle/builtin.h"
#include "curdle/curdle.h"
#include "curdle/values/ComptimeType.h"
#include "project/GlobalContext.h"
#include "curdle/values/ComptimeFunctionSet.h"
#include "curdle/values/ComptimeEnumLiteral.h"
#include "curdle/types/FunctionPointerType.h"

namespace cheese::curdle {

    std::map<std::string, Builtin> builtins;

    template<typename T>
    requires std::is_base_of_v<Type, T>
    static gcref<ComptimeValue> create_from_type(cheese::project::GlobalContext *gctx, T *ref) {
        auto type = new ComptimeType{gctx, static_cast<Type *>(ref)};
        return {gctx->gc, type};
    }

    gcref<ComptimeValue>
    type_builtin(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
                 RuntimeContext *rctx) {
        if (arguments.size() != 1) {
            throw LocalizedCurdleError(
                    "Attempting to use $Type w/ the wrong number of arguments, it only takes one argument", location,
                    error::ErrorCode::BadBuiltinCall);
        }
        if (!rctx) {
            return create_from_type(cctx->globalContext, cctx->exec(arguments[0], rctx)->type);
        } else {
            return create_from_type(cctx->globalContext, rctx->get_type(arguments[0]).get());
        }
    }

    BUILTIN("Type", type_builtin)

    gcref<ComptimeValue>
    peer_builtin(Coordinate location, std::vector<parser::Node *> arguments, ComptimeContext *cctx,
                 RuntimeContext *rctx) {
        std::vector<Type *> all_types;
        std::vector<gcref<ComptimeValue>> local_references;
        for (auto argument: arguments) {
            auto type_value = cctx->exec(argument, rctx);
            if (auto correct = dynamic_cast<ComptimeType *>(type_value.get()); correct) {
                all_types.push_back(correct->typeValue);
            } else {
                throw LocalizedCurdleError(
                        "Attempting to use $Peer w/ an argument of type " + type_value->type->to_string(),
                        argument->location, error::ErrorCode::BadBuiltinCall);
            }
            local_references.push_back(std::move(type_value));
        }
        return create_from_type(cctx->globalContext, peer_type(all_types, cctx->globalContext).value);
    }

    BUILTIN("Peer", peer_builtin)

    bacteria::BacteriaPtr
    fn_ptr_builtin(Coordinate location, LocalContext *localContext, std::vector<parser::Node *> arguments) {
        std::vector<gcref<managed_object>> _keepInScope;

        auto runtimeContext = localContext->runtime;
        auto comptimeContext = runtimeContext->comptime;
        auto globalContext = comptimeContext->globalContext;
        if (arguments.empty()) {
            throw LocalizedCurdleError("Attempting to use $fnPtr w/ the wrong number of arguments",
                                       arguments[0]->location,
                                       error::ErrorCode::BadBuiltinCall);
        }
        auto first_arg = comptimeContext->exec(arguments[0], runtimeContext);
        auto as_function = dynamic_cast<ComptimeFunctionSet *>(first_arg.get());
        if (!as_function)
            throw LocalizedCurdleError(
                    "Attempting to use $fnPtr with a first argument that is not a function but rather a " +
                    first_arg->type->to_string(), location, error::ErrorCode::BadBuiltinCall);
        std::vector<PassedFunctionArgument> passedArguments;
        for (int i = 1; i < arguments.size(); i++) {
            // Now here is where we just use comptime enum literal values, that aren't of any type, such that it is executed at comptime
            // .Type(...) -> This argument gets treated as a type
            // .Value(...) -> This argument gets treated as a value
            // Hmm, maybe I should define my own enum type for this result at some point to make sure it's encoded correctly
            auto value = comptimeContext->exec(arguments[i], runtimeContext);
            auto value_ptr = value.get();
            _keepInScope.emplace_back(std::move(value));
            if (auto enum_value = dynamic_cast<ComptimeEnumLiteral *>(value_ptr); enum_value) {
                if (enum_value->tuple_fields.size() != 1) {
                    throw LocalizedCurdleError(
                            "Arguments to $fnPtr must have only one tuple field",
                            arguments[i]->location,
                            error::ErrorCode::BadBuiltinCall
                    );
                }
                auto subValue = enum_value->tuple_fields[0];
                if (enum_value->value == "Type") {
                    if (auto type_value = dynamic_cast<ComptimeType *>(subValue); type_value) {
                        passedArguments.push_back(PassedFunctionArgument{
                                true,
                                nullptr,
                                type_value->typeValue
                        });
                    } else {
                        throw LocalizedCurdleError(
                                "The field of .Type must be a type",
                                arguments[i]->location,
                                error::ErrorCode::BadBuiltinCall
                        );
                    }
                } else if (enum_value->value == "Value") {
                    passedArguments.push_back(PassedFunctionArgument{false, subValue, nullptr});
                } else {
                    throw LocalizedCurdleError(
                            "Arguments to $fnPtr must be enums of the form .Type(...) or .Value(...)",
                            arguments[i]->location,
                            error::ErrorCode::BadBuiltinCall
                    );
                }
            } else {
                throw LocalizedCurdleError(
                        "Arguments to $fnPtr must be of enums the form .Type(...) or .Value(...)",
                        arguments[i]->location,
                        error::ErrorCode::BadBuiltinCall
                );
            }
        }
        auto fn = as_function->set->get(passedArguments);
        return std::make_unique<bacteria::nodes::ReferenceNode>(location,
                                                                std::make_unique<bacteria::nodes::ValueReference>(
                                                                        arguments[0]->location, fn->mangled_name));
    }

    gcref<Type>
    fn_ptr_type(Coordinate location, LocalContext *localContext, std::vector<parser::Node *> arguments) {
        std::vector<gcref<managed_object>> _keepInScope;

        auto runtimeContext = localContext->runtime;
        auto comptimeContext = runtimeContext->comptime;
        auto globalContext = comptimeContext->globalContext;
        if (arguments.empty()) {
            throw LocalizedCurdleError("Attempting to use $fnPtr w/ the wrong number of arguments",
                                       arguments[0]->location,
                                       error::ErrorCode::BadBuiltinCall);
        }
        auto first_arg = comptimeContext->exec(arguments[0], runtimeContext);
        auto as_function = dynamic_cast<ComptimeFunctionSet *>(first_arg.get());
        if (!as_function)
            throw LocalizedCurdleError(
                    "Attempting to use $fnPtr with a first argument that is not a function but rather a " +
                    first_arg->type->to_string(), location, error::ErrorCode::BadBuiltinCall);
        std::vector<PassedFunctionArgument> passedArguments;
        for (int i = 1; i < arguments.size(); i++) {
            // Now here is where we just use comptime enum literal values, that aren't of any type, such that it is executed at comptime
            // .Type(...) -> This argument gets treated as a type
            // .Value(...) -> This argument gets treated as a value
            // Hmm, maybe I should define my own enum type for this result at some point to make sure it's encoded correctly
            auto value = comptimeContext->exec(arguments[i], runtimeContext);
            auto value_ptr = value.get();
            _keepInScope.emplace_back(std::move(value));
            if (auto enum_value = dynamic_cast<ComptimeEnumLiteral *>(value_ptr); enum_value) {
                if (enum_value->tuple_fields.size() != 1) {
                    throw LocalizedCurdleError(
                            "Arguments to $fnPtr must have only one tuple field",
                            arguments[i]->location,
                            error::ErrorCode::BadBuiltinCall
                    );
                }
                auto subValue = enum_value->tuple_fields[0];
                if (enum_value->value == "Type") {
                    if (auto type_value = dynamic_cast<ComptimeType *>(subValue); type_value) {
                        passedArguments.push_back(PassedFunctionArgument{
                                true,
                                nullptr,
                                type_value->typeValue
                        });
                    } else {
                        throw LocalizedCurdleError(
                                "The field of .Type must be a type",
                                arguments[i]->location,
                                error::ErrorCode::BadBuiltinCall
                        );
                    }
                } else if (enum_value->value == "Value") {
                    passedArguments.push_back(PassedFunctionArgument{false, subValue, nullptr});
                } else {
                    throw LocalizedCurdleError(
                            "Arguments to $fnPtr must be enums of the form .Type(...) or .Value(...)",
                            arguments[i]->location,
                            error::ErrorCode::BadBuiltinCall
                    );
                }
            } else {
                throw LocalizedCurdleError(
                        "Arguments to $fnPtr must be of enums the form .Type(...) or .Value(...)",
                        arguments[i]->location,
                        error::ErrorCode::BadBuiltinCall
                );
            }
        }
        auto fn = as_function->set->get(passedArguments);
        auto fnPtr = globalContext->gc.gcnew<FunctionPointerType>();
        fnPtr->return_type = fn->returnType;
        for (const auto &arg: fn->arguments) {
            if (arg.is_type) {
                fnPtr->argument_types.push_back(arg.type);
            }
        }
        return fnPtr;
    }

    BUILTIN2("fnPtr", fn_ptr_builtin, fn_ptr_type)

    BadBuiltinCall::BadBuiltinCall(const std::string &message) : runtime_error(message) {}
}