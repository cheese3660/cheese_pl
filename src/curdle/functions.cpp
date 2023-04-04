//
// Created by Lexi Allen on 4/1/2023.
//


#include <utility>

#include "curdle/functions.h"
#include "curdle/GlobalContext.h"

namespace cheese::curdle {

    void FunctionTemplate::define() {
        if (defined) return;
        auto &gc = ctx->globalContext->gc;
        auto global = ctx->globalContext;
        // This is where we get all the information on each template (just argument types and return types)
        // We have to switch on the type of node to define everything correctly
        parser::NodeList *args = nullptr;
        parser::NodePtr ret_type;
        auto true_ptr = ptr.get();
        if (auto as_fn = dynamic_cast<parser::nodes::Function *>(true_ptr); as_fn) {
            args = &as_fn->arguments;
            ret_type = as_fn->return_type;
        } else if (auto as_gen = dynamic_cast<parser::nodes::Generator *>(true_ptr); as_gen) {
            args = &as_gen->arguments;
            ret_type = as_gen->return_type;
        } else if (auto as_op = dynamic_cast<parser::nodes::Operator *>(true_ptr); as_op) {
            args = &as_op->arguments;
            ret_type = as_op->return_type;
        }
        parser::NodeList &args_ref = *args;

        size_t unnamed_arg_index = 0;
        for (auto &argument: args_ref) {
            auto arg_ptr = argument.get();
            if (auto as_self = dynamic_cast<parser::nodes::Self *>(arg_ptr); as_self) {
                auto ref = ctx->currentStructure;
                auto ref_type = gc.gcnew<ReferenceType>(static_cast<Type *>(ref), false);
                auto arg = FunctionTemplateArgument{
                        ref_type,
                        "self",
                        ref->get_comptimeness()
                };
                arguments.push_back(arg);
            } else if (auto as_const_self = dynamic_cast<parser::nodes::Self *>(arg_ptr); as_const_self) {
                auto ref = ctx->currentStructure;
                auto ref_type = gc.gcnew<ReferenceType>(static_cast<Type *>(ref), true);
                auto arg = FunctionTemplateArgument{
                        ref_type,
                        "self",
                        ref->get_comptimeness()
                };
                arguments.push_back(arg);
            } else if (auto as_argument = dynamic_cast<parser::nodes::Argument *>(arg_ptr); as_argument) {
                auto value = ctx->exec(as_argument->type);
                if (auto t = dynamic_cast<ComptimeType *>(value.value); t) {
                    auto arg = FunctionTemplateArgument{
                            t->typeValue,
                            as_argument->name.value_or("$" + std::to_string(unnamed_arg_index++)),
                            as_argument->comptime ? Comptimeness::Comptime : t->typeValue->get_comptimeness()
                    };
                } else {
                    global->raise("Expected a type value for an argument", argument->location,
                                  error::ErrorCode::ExpectedType);
                }
            }
        }
        auto ret_value = ctx->exec(ret_type);
        if (auto t = dynamic_cast<ComptimeType *>(ret_value.value); t) {
            return_type = t->typeValue;
        } else {
            global->raise("Expected a type value for a return type", ret_type->location,
                          error::ErrorCode::ExpectedType);
        }
        // I think we should release the AST node here, but the AST node will be referenced to the end of time by a cache
        defined = true;
    }

    void FunctionTemplate::mark_references() {
        ctx->mark();
        for (auto &argument: arguments) {
            argument.type->mark();
        }
        if (return_type != nullptr)
            return_type->mark();
    }

    FunctionTemplate::FunctionTemplate(ComptimeContext *pContext, std::shared_ptr<parser::Node> sharedPtr) {
        ctx = pContext;
        ptr = std::move(sharedPtr);
        defined = false;
    }

    std::vector<Comptimeness> FunctionTemplate::ctime_values() {
        std::vector<Comptimeness> result{};
        for (auto &arg: arguments) {
            result.push_back(arg.comptimeness);
        }
        return result;
    }

    void FunctionSet::mark_references() {
        for (auto &temp: templates) {
            temp->mark();
        }
    }
}