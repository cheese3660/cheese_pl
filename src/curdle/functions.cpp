//
// Created by Lexi Allen on 4/1/2023.
//


#include <utility>

#include "curdle/functions.h"
#include "project/GlobalContext.h"
#include "curdle/runtime.h"
#include <typeinfo>
#include "curdle/names.h"
#include "curdle/curdle.h"
#include "bacteria/nodes/receiver_nodes.h"
#include "curdle/types/ReferenceType.h"
#include "curdle/values/ComptimeType.h"
#include "curdle/types/AnyType.h"

namespace cheese::curdle {

    void FunctionTemplate::mark_references() {
        ctx->mark();
        for (auto concrete: concrete_functions) {
            concrete->mark();
        }
    }

    FunctionTemplate::FunctionTemplate(ComptimeContext *pContext, std::shared_ptr<parser::Node> sharedPtr) {
        ctx = pContext;
        ptr = std::move(sharedPtr);
    }

    FunctionInfo FunctionTemplate::get_info_for_arguments(const std::vector<PassedFunctionArgument> &arguments,
                                                          bool any_zero) const {
        auto &gc = ctx->globalContext->gc;
        auto fctx = gc.gcnew<ComptimeContext>(ctx);
        auto rctx = gc.gcnew<RuntimeContext>(fctx, fctx->currentStructure);
        parser::NodeList *args_list = nullptr;
        parser::NodePtr ret_type;
        bool force_comptime;
        auto true_ptr = ptr.get();
        if (auto as_fn = dynamic_cast<parser::nodes::Function *>(true_ptr); as_fn) {
            args_list = &as_fn->arguments;
            ret_type = as_fn->return_type;
            force_comptime = as_fn->flags.comptime;
        } else if (auto as_gen = dynamic_cast<parser::nodes::Generator *>(true_ptr); as_gen) {
            args_list = &as_gen->arguments;
            ret_type = as_gen->return_type;
            force_comptime = as_gen->flags.comptime;
        } else if (auto as_op = dynamic_cast<parser::nodes::Operator *>(true_ptr); as_op) {
            args_list = &as_op->arguments;
            ret_type = as_op->return_type;
            force_comptime = as_op->flags.comptime;
        }
        auto no_match = FunctionInfo{
                {},
                -1,
                gcref<Type>(gc, nullptr)
        };
        if (args_list->size() != arguments.size()) {
            //Return a closeness of -1
            return no_match;
        }
        std::vector<FunctionTemplateArgument> args;
        args.reserve(args_list->size());
        // For each argument, check the "castiness" of the argument, and add the value for each type of castiness to closeness
        // 0 is exact
        // 65536 are cast operators once those are implemented
        // 65537 are "any" casts
        // 1-65535 is widening cast (same sign) (defined by the number of bits difference), anything else results in an error and you must explicitly cast
        int closeness = 0;
        size_t unnamed_arg_index = 0;
        for (int i = 0; i < arguments.size(); i++) {
            auto t1 = arguments[i].type;
            auto a2 = args_list->at(i).get();
            if (auto as_self = dynamic_cast<parser::nodes::Self *>(a2); as_self) {
                auto ref = ctx->currentStructure;
                auto ref_type = gc.gcnew<ReferenceType>(static_cast<Type *>(ref), false);
                if (ref_type->get_comptimeness() == Comptimeness::Comptime || force_comptime) {
                    if (arguments[i].is_type) return no_match;
                    fctx->comptimeVariables["self"] = gc.gcnew<ComptimeVariable>(ref, arguments[i].value);
                } else {
                    rctx->variables["self"] = RuntimeVariableInfo{
                            true,
                            "self",
                            ref_type,
                    };
                }
                auto comparison = ref_type->compare(arguments[i].type);
                if (comparison == -1) {
                    return no_match;
                } else {
                    closeness += comparison;
                }
                auto arg = FunctionTemplateArgument{
                        std::move(ref_type),
                        "self",
                        force_comptime ? Comptimeness::Comptime : ref->get_comptimeness()
                };
                args.push_back(std::move(arg));
            } else if (auto as_const_self = dynamic_cast<parser::nodes::ConstSelf *>(a2); as_const_self) {
                auto ref = ctx->currentStructure;
                auto ref_type = gc.gcnew<ReferenceType>(static_cast<Type *>(ref), true);
                if (ref_type->get_comptimeness() == Comptimeness::Comptime || force_comptime) {
                    if (arguments[i].is_type) return no_match;
                    fctx->comptimeVariables["self"] = gc.gcnew<ComptimeVariable>(ref, arguments[i].value);
                } else {
                    rctx->variables["self"] = RuntimeVariableInfo{
                            true,
                            "self",
                            ref_type,
                    };
                }
                auto comparison = ref_type->compare(arguments[i].type);
                if (comparison == -1) {
                    return no_match;
                } else {
                    closeness += comparison;
                }
                auto arg = FunctionTemplateArgument{
                        std::move(ref_type),
                        "self",
                        force_comptime ? Comptimeness::Comptime : ref->get_comptimeness()
                };
                args.push_back(std::move(arg));
            } else if (auto as_argument = dynamic_cast<parser::nodes::Argument *>(a2); as_argument) {
                auto value = fctx->exec(as_argument->type, rctx);
                if (auto t = dynamic_cast<ComptimeType *>(value.value); t) {
                    auto name = as_argument->name.value_or("$" + std::to_string(unnamed_arg_index++));
                    if (auto as_any = dynamic_cast<AnyType *>(t->typeValue); as_any) {
                        if (!any_zero) {
                            closeness += 131072;
                        }
                        if ((force_comptime || as_argument->comptime) && arguments[i].is_type) {
                            return no_match;
                        }
                        if (force_comptime || as_argument->comptime) {
                            fctx->comptimeVariables[name] = gc.gcnew<ComptimeVariable>(as_any, arguments[i].value);
                            args.push_back({
                                                   gcref<Type>(gc, as_any),
                                                   name,
                                                   Comptimeness::Comptime
                                           });
                        } else {
                            rctx->variables[name] = RuntimeVariableInfo{
                                    true,
                                    name,
                                    arguments[i].type
                            };
                            args.push_back({
                                                   gcref<Type>(gc, arguments[i].type),
                                                   name,
                                                   arguments[i].type->get_comptimeness()
                                           });
                        }
                    } else {
                        auto tv = t->typeValue;
                        auto cv = tv->compare(t1);
                        if (cv == -1) {
                            return no_match;
                        }
                        bool comptime = force_comptime || as_argument->comptime ||
                                        tv->get_comptimeness() == Comptimeness::Comptime;
                        if (comptime && arguments[i].is_type) {
                            return no_match;
                        }
                        if (comptime) {
                            if (cv == 0) {
                                fctx->comptimeVariables[name] = gc.gcnew<ComptimeVariable>(tv, arguments[i].value);
                            } else {
                                // TODO: cast comptime values
                                NOT_IMPL_FOR("comptime casting");
                            }
                        } else {
                            rctx->variables[name] = RuntimeVariableInfo{
                                    true,
                                    name,
                                    tv
                            };
                        }
                        args.push_back({
                                               gcref<Type>(gc, tv),
                                               name,
                                               comptime ? Comptimeness::Comptime : Comptimeness::Runtime
                                       });
                    }
                } else {
                    ctx->globalContext->raise("Expected a type value for an argument", a2->location,
                                              error::ErrorCode::ExpectedType);
                }
            }
        }
        // Auto return type deduction will not exist *yet*, in the future though it will, but that's a whole can o worms
        auto ret = fctx->exec(ret_type, rctx);
        if (auto t = dynamic_cast<ComptimeType *>(ret.value); t) {
            // If we are returning a comptime only value then force compile time deduction of the arguments
            if (t->typeValue->get_comptimeness() == Comptimeness::Comptime) {
                for (auto &argument: args) {
                    if (argument.comptimeness != Comptimeness::Comptime) {
                        return no_match;
                    }
                }
            }
            return FunctionInfo{
                    std::move(args),
                    closeness,
                    gcref<Type>(gc, t->typeValue)
            };
        } else {
            ctx->globalContext->raise("Expected a type value for return type", ret_type->location,
                                      error::ErrorCode::ExpectedType);
            return no_match;
        }
    }

    ConcreteFunction *
    FunctionTemplate::get(const std::vector<PassedFunctionArgument> &arguments) {
        auto info = get_info_for_arguments(arguments, true);
        auto expected_closeness = info.closeness;
        if (expected_closeness == -1) {
//            ctx->globalContext->raise("Mismatched function call, argument types or number do not match up", call_loc,
//                                      error::ErrorCode::MismatchedFunctionCall);
            throw CurdleError("Mismatched function call, argument types or number do not match up",
                              error::ErrorCode::MismatchedFunctionCall);
        }
        // Lets first check each concrete function we have defined so far :3
        ConcreteFunction *ret_val = nullptr;
        int count = 0;
        for (auto concrete_function: concrete_functions) {
            auto confined_closeness = concrete_function->closeness(arguments);
            if (confined_closeness != -1 && confined_closeness <= expected_closeness) {
                ret_val = concrete_function;
                count++;
            }
        }
        if (count == 1) {
            return ret_val;
        } else if (count > 1) {
            // We should just throw here
//            ctx->globalContext->raise("Ambiguous function call for given arguments", call_loc,
//                                      error::ErrorCode::AmbiguousFunctionCall);
//            return nullptr;
            throw CurdleError("Ambiguous function call for given arguments", error::ErrorCode::AmbiguousFunctionCall);
        }
        // Now here is where things get funky, where we have to generate a function definition since none other fits
        auto &gc = ctx->globalContext->gc;
        auto fctx = gc.gcnew<ComptimeContext>(ctx);
        auto rctx = gc.gcnew<RuntimeContext>(fctx, fctx->currentStructure);
        std::vector<PassedFunctionArgument> true_arguments{};
        for (int i = 0; i < arguments.size(); i++) {
            if (info.arguments[i].comptimeness == Comptimeness::Comptime) {
                if (info.arguments[i].type->compare(arguments[i].type) != 0) {
                    // We shall do a cast here
                    NOT_IMPL_FOR("comptime casting");
                } else {
                    fctx->comptimeVariables[info.arguments[i].name] = gc.gcnew<ComptimeVariable>(info.arguments[i].type,
                                                                                                 arguments[i].value);
                    true_arguments.push_back(arguments[i]);
                }
            } else {
                rctx->variables[info.arguments[i].name] = RuntimeVariableInfo{true, info.arguments[i].name,
                                                                              info.arguments[i].type};
                true_arguments.push_back(PassedFunctionArgument{true, nullptr, info.arguments[i].type});
            }
        }
        auto &ret_type = info.return_type;
        parser::NodePtr body_ptr;
        bool force_comptime;
        bool external;
        bool entry;
        bool generator{false};
        std::string name;
        auto true_ptr = ptr.get();
        if (auto as_fn = dynamic_cast<parser::nodes::Function *>(true_ptr); as_fn) {
            force_comptime = as_fn->flags.comptime;
            external = as_fn->flags.exter || as_fn->flags.entry;
            entry = as_fn->flags.entry;
            name = as_fn->name;
            body_ptr = as_fn->body;
        } else if (auto as_gen = dynamic_cast<parser::nodes::Generator *>(true_ptr); as_gen) {
            force_comptime = as_gen->flags.comptime;
            external = as_gen->flags.exter;
            name = as_gen->name;
            body_ptr = as_gen->body;
            generator = true;
        } else if (auto as_op = dynamic_cast<parser::nodes::Operator *>(true_ptr); as_op) {
            force_comptime = as_op->flags.comptime;
            external = false;
            name = "operator" + as_op->op;
            body_ptr = as_op->body;
        }
        for (const auto &funct: concrete_functions) {
            if (funct->matches(true_arguments)) return funct;
        }
        if (!external) {
            name = combine_names(ctx->currentStructure->name, name);
        }
        bool comptime_only = force_comptime;
        if (ret_type->get_comptimeness() == Comptimeness::Comptime) {
            comptime_only = true;
        }
        rctx->functionReturnType = ret_type;

        std::vector<std::string> rtime_names;
        for (auto &argument: info.arguments) {
            if (argument.comptimeness == Comptimeness::Runtime) {
                rtime_names.push_back(argument.name);
            }
        }

        auto new_function = gc.gcnew<ConcreteFunction>(name, true_arguments, ret_type, comptime_only, external, entry);
        concrete_functions.push_back(new_function);
        new_function->generate_code(fctx, rctx,
                                    external, std::move(body_ptr), generator, rtime_names);

        return new_function;
    }

    ConcreteFunction *FunctionTemplate::get() {
        // Essentially it will do some magic
        auto &gctx = ctx->globalContext;
        auto &gc = gctx->gc;
        if (concrete_functions.empty()) {
            // We have to generate a new function
            std::vector<gcref<managed_object>> _keepInScope;
            auto fctx = gc.gcnew<ComptimeContext>(ctx);
            auto rctx = gc.gcnew<RuntimeContext>(fctx, fctx->currentStructure);
            std::vector<PassedFunctionArgument> true_arguments{};
            parser::NodeList args_list;
            parser::NodePtr ret_val;
            parser::NodePtr body_ptr;
            bool force_comptime;
            bool external;
            bool entry;
            bool generator{false};
            std::string name;
            auto true_ptr = ptr.get();
            if (auto as_fn = dynamic_cast<parser::nodes::Function *>(true_ptr); as_fn) {
                force_comptime = as_fn->flags.comptime;
                external = as_fn->flags.exter || as_fn->flags.entry;
                entry = as_fn->flags.entry;
                name = as_fn->name;
                body_ptr = as_fn->body;
                args_list = as_fn->arguments;
                ret_val = as_fn->return_type;
            } else if (auto as_gen = dynamic_cast<parser::nodes::Generator *>(true_ptr); as_gen) {
                force_comptime = as_gen->flags.comptime;
                external = as_gen->flags.exter;
                name = as_gen->name;
                body_ptr = as_gen->body;
                generator = true;
                args_list = as_gen->arguments;
                ret_val = as_gen->return_type;
                NOT_IMPL_FOR("generators");
            } else if (auto as_op = dynamic_cast<parser::nodes::Operator *>(true_ptr); as_op) {
                force_comptime = as_op->flags.comptime;
                external = false;
                name = "operator" + as_op->op;
                body_ptr = as_op->body;
                args_list = as_fn->arguments;
                ret_val = as_fn->return_type;
            }
            int next_arg = 0;
            std::vector<std::string> rtime_names;
            for (auto &arg: args_list) {
                auto as_arg = dynamic_cast<parser::nodes::Argument *>(arg.get());
                auto ty = fctx->exec(as_arg->type, rctx);
                auto arg_name = as_arg->name.has_value() ? as_arg->name.value() : "_" + std::to_string(next_arg++);
                if (auto as_ty = dynamic_cast<ComptimeType *>(ty.get()); as_ty) {
                    rctx->variables[arg_name] = RuntimeVariableInfo{
                            true,
                            arg_name,
                            as_ty->typeValue
                    };
                    true_arguments.emplace_back(true, nullptr, as_ty->typeValue);
                    rtime_names.push_back(arg_name);
                } else {
                    throw LocalizedCurdleError{
                            "Expected type",
                            arg->location,
                            error::ErrorCode::ExpectedType
                    };
                }
                _keepInScope.emplace_back(std::move(ty));
            }
            for (const auto &funct: concrete_functions) {
                if (funct->matches(true_arguments)) return funct;
            }
            auto ret_ty_val = fctx->exec(ret_val, rctx);
            Type *ret_ty;
            if (auto as_ty = dynamic_cast<ComptimeType *>(ret_ty_val.get()); as_ty) {
                ret_ty = as_ty->typeValue;
            } else {
                throw LocalizedCurdleError{
                        "Expected type",
                        ret_val->location,
                        error::ErrorCode::ExpectedType
                };
            }
            rctx->functionReturnType = ret_ty;
            auto new_function = gc.gcnew<ConcreteFunction>(name, true_arguments, ret_ty, false, external,
                                                           entry);
            concrete_functions.push_back(new_function);
            new_function->generate_code(fctx, rctx,
                                        external, std::move(body_ptr), generator, rtime_names);
            return new_function;
        } else if (concrete_functions.size() == 1) {
            return concrete_functions[0];
        } else if (concrete_functions.size() > 1) {
            throw CurdleError{
                    "Ambiguous function matching",
                    error::ErrorCode::AmbiguousFunctionCall
            };
        }
    }

    void FunctionSet::mark_references() {
        for (auto &temp: templates) {
            temp->mark();
        }
    }

    std::string get_arg_name_list(const std::vector<PassedFunctionArgument> &arguments) {
        std::stringstream ss;
        for (int i = 0; i < arguments.size(); i++) {
            if (arguments[i].is_type) {
                ss << "(type) " << arguments[i].type->to_string();
            } else {
                ss << "(value) " << arguments[i].value->to_string();
            }
            if (i < arguments.size() - 1) {
                ss << ", ";
            }
            if (i > 0 && i == arguments.size() - 2) {
                ss << "and ";
            }
        }
        return ss.str();
    }

    ConcreteFunction *FunctionSet::get(const std::vector<PassedFunctionArgument> &arguments) {
        int closest_closeness = std::numeric_limits<int>::max();
        FunctionTemplate *closest_function = nullptr;
        for (auto templ: templates) {
            auto info = templ->get_info_for_arguments(arguments);
            if (info.closeness >= 0 && info.closeness < closest_closeness) {
                closest_function = templ;
            }
        }
        if (closest_function == nullptr) {
            throw CurdleError{
                    "No Overload: No overload found for the following argument set: " + get_arg_name_list(arguments),
                    error::ErrorCode::NoOverloadFound
            };
        }
        return closest_function->get(arguments);
    }

    ConcreteFunction *FunctionSet::get() {
        if (templates.size() != 1) {
            throw CurdleError{
                    "Ambiguous function matching",
                    error::ErrorCode::AmbiguousFunctionCall
            };
        } else {
            return templates[0]->get();
        }
    }

    IncorrectCallException::IncorrectCallException() = default;

    const char *IncorrectCallException::what() const noexcept {
        return "Invalid function call";
    }

    int ConcreteFunction::closeness(const std::vector<PassedFunctionArgument> &args) {
        if (args.size() != arguments.size()) {
            return -1;
        }
        int closeness = 0;
        for (int i = 0; i < args.size(); i++) {
            if (!arguments[i].is_type && args[i].is_type) {
                return -1;
            }
            auto comparison = arguments[i].type->compare(args[i].type);
            if (comparison == -1) {
                return -1;
            }
            closeness += comparison;
            if (!arguments[i].is_type) {
                auto a = arguments[i].value;
                auto b = args[i].value;
                if (!a->is_same_as(b)) return -1;
            }
        }
        return closeness;
    }

    void ConcreteFunction::mark_references() {
        returnType->mark();
        for (auto &argument: arguments) {
            argument.type->mark();
            if (!argument.is_type) {
                argument.value->mark();
            }
        }
        if (returned_value != nullptr) {
            returned_value->mark();
        }
    }

    void ConcreteFunction::generate_code(ComptimeContext *cctx, RuntimeContext *rctx, bool external,
                                         parser::NodePtr body_ptr, bool is_generator,
                                         const std::vector<std::string> &rtime_names) {

        if (is_comptime_only) {
            NOT_IMPL_FOR("compile time");
        } else {
            // Now we translate the function body
            if (is_generator) {
                NOT_IMPL_FOR("generators");
            } else {
                std::vector<bacteria::FunctionArgument> bacteria_args{};
                size_t rtime_index = 0;
                for (auto &argument: arguments) {
                    if (argument.is_type) {
                        bacteria_args.push_back(bacteria::FunctionArgument{
                                argument.type->get_cached_type(cctx->globalContext->global_receiver.get()),
                                rtime_names[rtime_index++],
                        });
                    }
                }
                auto node = (new bacteria::nodes::Function{body_ptr->location, mangled_name, bacteria_args,
                                                           returnType->get_cached_type(
                                                                   cctx->globalContext->global_receiver.get())})->get();
                rctx->local_reciever = dynamic_cast<bacteria::BacteriaReceiver *>(node.get());
                cctx->globalContext->global_receiver->receive(std::move(node));
            }
            translate_function_body(rctx, std::move(body_ptr));
        }
    }

    ConcreteFunction::ConcreteFunction(std::string path, const std::vector<PassedFunctionArgument> &arguments,
                                       Type *returnType, bool comptimeOnly, bool external, bool entry) {
        // Comptime argument typenames to the name mangler get passed like such
        // TYPENAME$VALUE
        is_comptime_only = comptimeOnly;
        this->returnType = returnType;
        returned_value = nullptr;
        if (comptimeOnly) {
            mangled_name = "";
            NOT_IMPL_FOR("compile time");
        } else if (entry) {
            mangled_name = "main"; // Name it main, as global constructors do exist
        } else {
            // Time to do a lot of stuff just to mangle the name
            std::vector<std::string> args;
            for (auto &argument: arguments) {
                if (argument.is_type) {
                    args.push_back(argument.type->to_string());
                } else {
                    args.push_back(argument.type->to_string() + '$' + argument.value->to_string());
                }
            }
            mangled_name = mangle(std::move(path), args, external);
        }
        this->arguments = std::move(arguments);
    }

    bool ConcreteFunction::matches(const std::vector<PassedFunctionArgument> &otherArgs) {
        if (arguments.size() != otherArgs.size()) return false;
        std::size_t index{0};
        for (const auto &argument: arguments) {
            if (argument.is_type && otherArgs[index].is_type) {
                if (argument.type->compare(otherArgs[index].type) != 0) return false;
            } else if (!argument.is_type && !otherArgs[index].is_type) {
                if (!argument.value->is_same_as(otherArgs[index].value)) return false;
            } else {
                return false;
            }
            index += 1;
        }
        return true;
    }


    ConcreteFunction *get_function(RuntimeContext *rctx, ComptimeContext *cctx, LocalContext *lctx,
                                   FunctionSet *function_set, parser::NodeList &arg_nodes) {
        std::vector<PassedFunctionArgument> arguments;
        // Store all the references so they will be deallocated at the *correct* time
        std::vector<gcref<ComptimeValue>> value_refs;
        std::vector<gcref<Type>> type_refs;
        for (const auto &node: arg_nodes) {
            if (auto arg_attempt = cctx->try_exec(node.get(),
                                                  rctx); arg_attempt.has_value()) {
                arguments.push_back(
                        PassedFunctionArgument{false, arg_attempt->get(), arg_attempt->get()->type});
                value_refs.push_back(std::move(arg_attempt.value()));
            } else {
                // We have to do a type deduction on this one :)
                auto arg_ty = lctx->get_type(node.get());
                arguments.push_back(PassedFunctionArgument{true, nullptr, arg_ty});
            }
        }
        // TODO:  Use exceptions so we don't have to pass call loc
        auto function = function_set->get(std::move(arguments));
        return function;
    }
}