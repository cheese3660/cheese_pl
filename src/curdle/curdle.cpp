//
// Created by Lexi Allen on 3/20/2023.
//
#include <utility>

#include "curdle/curdle.h"

#include "curdle/GlobalContext.h"

#include "parser/parser.h"

#include "parser/nodes/other_nodes.h"
#include "parser/nodes/single_member_nodes.h"
#include "parser/nodes/terminal_nodes.h"
#include "curdle/comptime.h"
#include "curdle/Interface.h"

#include "error.h"

#include "bacteria/BacteriaNode.h"
#include "bacteria/nodes/expression_nodes.h"

using namespace cheese::memory::garbage_collection;

namespace cheese::curdle {


    void resolve_mixin(ComptimeContext *ctx, parser::nodes::Mixin *mixin) {

    }

    // Likely going to need more parameters, but this should be fine
    gcref<Structure>
    translate_structure(ComptimeContext *ctx, parser::nodes::Structure *structure_node) {
        auto &gc = ctx->globalContext->gc;
        auto &global = ctx->globalContext;
        auto structure_ref = gc.gcnew<Structure>();
        auto ref = structure_ref.get();
        auto localCtx = gc.gcnew<ComptimeContext>(ctx, ref);
        structure_ref->containedContext = localCtx;
        structure_ref->name = "";
        if (structure_node->is_tuple) {
            structure_ref->is_tuple = true;
            size_t field_index = 0;
            for (auto &child: structure_node->children) {
                auto val = localCtx->exec(child);
                auto type = val->type;
                if (dynamic_cast<TypeType *>(type)) {
                    auto value = dynamic_cast<ComptimeType *>(val.get());
                    if (!value) {
                        global->raise("Expected a type value for a field", child->location,
                                      error::ErrorCode::ExpectedType);
                        continue;
                    }
                    auto field_name = "_" + std::to_string(field_index++);
                    auto field = StructureField{field_name, value->typeValue};
                    structure_ref->fields.push_back(std::move(field));
                } else {
                    global->raise("Expected a type value for a field", child->location,
                                  error::ErrorCode::ExpectedType);
                }
            }
        } else {
            // Order of implementation
            // 1. Resolve interfaces
            // 2. Read lazies (this includes methods)
            // 3. Run comptime blocks
            // 4. Resolve field types
            // 5. Resolve mixins

            // Resolve interfaces
            for (auto &child: structure_node->interfaces) {
                auto val = localCtx->exec(child);
                // Now we must add the interface type to our types
                auto type = val->type;
                if (dynamic_cast<TypeType *>(type)) {
                    auto value = dynamic_cast<Interface *>(val.get());
                    if (!value) {
                        global->raise("Expected a type value for a field", child->location,
                                      error::ErrorCode::ExpectedType);
                        continue;
                    }
                    structure_ref->interfaces.push_back(value);
                } else {
                    global->raise("Expected an interface", child->location, error::ErrorCode::ExpectedInterface);
                }
            }

            // The locations will still exist
            std::vector<parser::nodes::Comptime *> comptime_blocks;
            std::vector<parser::nodes::Mixin *> mixins;
            std::vector<parser::nodes::Field *> fields;


            // Read lazies, and grab comptime blocks
            for (auto &child: structure_node->children) {
                auto ptr = child.get();
                if (auto as_comptime = dynamic_cast<parser::nodes::Comptime *>(ptr); as_comptime) {
                    comptime_blocks.push_back(as_comptime);
                } else if (auto as_mixin = dynamic_cast<parser::nodes::Mixin *>(ptr); as_mixin) {
                    mixins.push_back(as_mixin);
                } else if (auto as_field = dynamic_cast<parser::nodes::Field *>(ptr); as_field) {
                    fields.push_back(as_field);
                } else if (auto as_decl = dynamic_cast<parser::nodes::VariableDeclaration *>(ptr); as_decl) {
                    auto def = dynamic_cast<parser::nodes::VariableDefinition *>(as_decl->def.get());
                    structure_ref->lazies.push_back(new LazyValue{def->name, child});
                } else if (auto as_def = dynamic_cast<parser::nodes::VariableDefinition *>(ptr); as_def) {
                    structure_ref->lazies.push_back(new LazyValue{as_def->name, child});
                } else if (auto as_import = dynamic_cast<parser::nodes::Import *>(ptr); as_import) {
                    structure_ref->lazies.push_back(new LazyValue{as_import->name, child});
                } else if (auto as_op = dynamic_cast<parser::nodes::Operator *>(ptr); as_op) {
                    auto temp = gc.gcnew<FunctionTemplate>(localCtx.get(), child);
                    if (!structure_ref->function_sets.contains(as_op->op)) {
                        structure_ref->function_sets[as_op->op] = gc.gcnew<FunctionSet>();
                    }
                    structure_ref->function_sets[as_op->op]->templates.push_back(temp.get());
                } else if (auto as_fn = dynamic_cast<parser::nodes::Function *>(ptr); as_fn) {
                    auto temp = gc.gcnew<FunctionTemplate>(localCtx.get(), child);
                    auto tptr = temp.get();
                    if (as_fn->flags.entry) ctx->globalContext->entry_function = tptr;
                    if (!structure_ref->function_sets.contains(as_fn->name)) {
                        structure_ref->function_sets[as_fn->name] = gc.gcnew<FunctionSet>();
                    }
                    structure_ref->function_sets[as_fn->name]->templates.push_back(tptr);
                } else if (auto as_gen = dynamic_cast<parser::nodes::Generator *>(ptr); as_gen) {
                    auto temp = gc.gcnew<FunctionTemplate>(localCtx.get(), child);
                    if (!structure_ref->function_sets.contains(as_gen->name)) {
                        structure_ref->function_sets[as_gen->name] = gc.gcnew<FunctionSet>();
                    }
                    structure_ref->function_sets[as_gen->name]->templates.push_back(temp.get());
                }
            }

            // Resolve comptime blocks
            for (auto block: comptime_blocks) {
                // Discard the value because yes
                localCtx->exec(block);
            }

            // Resolve field types
            size_t unnamed_index = 0;
            for (auto field: fields) {
                auto val = localCtx->exec(field->type);
                if (auto type = dynamic_cast<TypeType *>(val->type); type) {
                    auto value = dynamic_cast<ComptimeType *>(val.get());

                    structure_ref->fields.push_back(StructureField{
                            field->name.value_or("$" + std::to_string(unnamed_index++)),
                            value->typeValue,
                            static_cast<bool>(field->flags.pub)
                    });
                }
            }

            // Resolve mixins
            for (auto mixin: mixins) {
                resolve_mixin(localCtx, mixin);
            }
        }

        return structure_ref;
    }

    void setup_root_structure(GlobalContext *ctx, const parser::NodePtr &ast_node) {
        garbage_collector &gc = ctx->gc;
        auto newContext = gc.gcnew<ComptimeContext>(ctx, ctx->project.root_path, ctx->project.folder);
        auto translated = translate_structure(newContext,
                                              dynamic_cast<parser::nodes::Structure *>(ast_node.get()));
        ctx->root_structure = translated.get();
    }


    std::unique_ptr<bacteria::BacteriaNode> curdle(const Project &project) {
        // First off create a structure for the main project file
        auto gc = garbage_collector{64}; // Lets do it every 64 iterations
        auto ctx = gc.gcnew<GlobalContext>(project, gc);
        gc.add_root_object(ctx);
        setup_root_structure(ctx, project.root_file);
        if (project.type == ProjectType::Application) {
            // We do a depth first search to find the entry point
            ctx->root_structure->search_entry();
            if (ctx->entry_function == nullptr) {
                error::raise_exiting_error("curdle", "no entrypoint found", project.root_file->location,
                                           error::ErrorCode::NoEntryPoint);
            }
            ctx->entry_function->get({}, project.root_file->location);
        }
        return ctx->global_reciever.release()->get();
    }

    bacteria::BacteriaPtr translate_expression(LocalContext *lctx, parser::NodePtr expr);

    bacteria::BacteriaPtr translate_tuple_call(LocalContext *lctx, parser::nodes::TupleCall *call) {
        // Now we try to do the fun thing and get the called object either as a comptime value, or runtime value
        // If it's a compile time value then that's *cool* it makes our jobs easier
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto empty_ctx = gc.gcnew<LocalContext>(rctx);
        if (auto attempt = cctx->try_exec(call->object.get(), rctx); attempt.has_value()) {
            auto ctime = attempt->get();
            if (auto function_set = dynamic_cast<ComptimeFunctionSet *>(ctime); function_set) {
                std::vector<PassedFunctionArgument> arguments;
                // Store all the references so they will be deallocated at the *correct* time
                std::vector<gcref<ComptimeValue>> value_refs;
                std::vector<gcref<Type>> type_refs;
                for (const auto &node: call->args) {
                    if (auto arg_attempt = cctx->try_exec(node.get(),
                                                          rctx); arg_attempt.has_value()) {
                        arguments.push_back(
                                PassedFunctionArgument{false, arg_attempt->get(), arg_attempt->get()->type});
                        value_refs.push_back(std::move(arg_attempt.value()));
                    } else {
                        // We have to do a type deduction on this one :)
                        auto arg_ty = empty_ctx->get_type(node.get());
                        arguments.push_back(PassedFunctionArgument{true, nullptr, arg_ty});
                    }
                }
                // TODO:  Use exceptions so we don't have to pass call loc
                auto function = function_set->set->get(std::move(arguments), call->location);
                std::vector<bacteria::BacteriaPtr> actual_arguments;
                for (int i = 0; i < call->args.size(); i++) {
                    if (function->arguments[i].is_type) {
                        auto &farg = function->arguments[i];
                        auto parg = call->args[i];
                        auto fctx = gc.gcnew<LocalContext>(rctx, farg.type);
                        auto compare = farg.type->compare(fctx->get_type(parg.get()));
                        if (compare == 0) {
                            actual_arguments.push_back(translate_expression(fctx, parg));
                        } else {
                            //TODO: more complex casting code once we add cast operators
                            actual_arguments.push_back(
                                    std::make_unique<bacteria::nodes::CastNode>(parg->location,
                                                                                translate_expression(fctx, parg),
                                                                                farg.type->get_cached_type()));
                        }
                    }
                }
                return std::make_unique<bacteria::nodes::NormalCallNode>(call->location, function->mangled_name,
                                                                         std::move(actual_arguments));
            }
            NOT_IMPL_FOR(typeid(*ctime).name());
        } else {
            NOT_IMPL_FOR("non compile time deductible functions");
        }
    }

    bacteria::BacteriaPtr translate_if_statement(LocalContext *lctx, parser::nodes::If *pIf) {
        // This is where we translate the if statement, which is going to involve a helluva lot of code
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto empty_ctx = gc.gcnew<LocalContext>(rctx);
        if (pIf->unwrap.has_value()) {
            // This is where we do a wierd translation for optionals, which are going to be a builtin type
            // But essentially it becomes
            // named_block: {
            //     let tmp = condition;
            //     <==(named_block) if tmp has value
            //         inner_named_block: {
            //             let unwrap = value of tmp
            //             <==(inner_named_block) body
            //         }
            //     else
            //        els
            // }
            // But for this we need to implement optionals :3
            NOT_IMPL_FOR("If unwrapping");
        } else {
            // Condition needs to be "truthified" depending on what type it is
            // If it is an integer, then it gets combined w/ != 0 for the "truthification"
            // If it is an optional, then it gets combined with the "has_value" operator
            // If it is a boolean then nothing is done
            // If it is comptime known then we don't even compile the if, we find the branch based on the value
            if (auto comptime_condition = cctx->try_exec(pIf->condition.get(),
                                                         rctx); comptime_condition.has_value()) {
                auto ctime = comptime_condition->get();
                // Now we need to define what needs to be done based off of the value
#define WHEN_CTIME_IS(type, name) if (auto name = dynamic_cast<type*>(ctime); name)
                WHEN_CTIME_IS(ComptimeInteger, pComptimeInteger) {
                    if (pComptimeInteger->value != 0) {
                        return (translate_expression(lctx, pIf->body));
                    } else {
                        // We have to add a "nop" for when an operation is just completely elided
                        if (pIf->els.has_value()) {
                            return translate_expression(lctx, pIf->els.value());
                        } else {
                            return std::make_unique<bacteria::nodes::Nop>(pIf->location);
                        }
                    }
                }

                cctx->globalContext->raise("Condition must be convertable to a boolean",
                                           pIf->condition->location, error::ErrorCode::InvalidCondition);
                return std::make_unique<bacteria::nodes::Nop>(pIf->location);
#undef WHEN_CTIME_IS
            } else {
                // Now we have to get a type
                auto ty = empty_ctx->get_type(pIf->condition.get());
                bacteria::BacteriaPtr condition_ptr;
#define TYPE_IS(type, name) (auto name = dynamic_cast<type*>(ty); name)
                // So many different types to go from here
                if TYPE_IS(IntegerType, pIntegerType) {
                    auto ictx = gc.gcnew<LocalContext>(empty_ctx, pIntegerType);
                    // Generate an expression tree that looks like the following
                    // x != 0 and make that the condition
                    condition_ptr = std::make_unique<bacteria::nodes::NotEqualNode>(pIf->condition->location,
                                                                                    translate_expression(ictx,
                                                                                                         pIf->condition),
                                                                                    std::make_unique<bacteria::nodes::IntegerLiteral>(
                                                                                            pIf->condition->location, 0,
                                                                                            pIntegerType->get_cached_type()));
                } else if TYPE_IS(BooleanType, pBooleanType) {
                    auto bctx = gc.gcnew<LocalContext>(empty_ctx, pBooleanType);
                    condition_ptr = translate_expression(bctx, pIf->condition);
                } else {
                    // Now we throw an error

                    cctx->globalContext->raise("Condition must be convertable to a boolean",
                                               pIf->condition->location, error::ErrorCode::InvalidCondition);
                    condition_ptr = std::make_unique<bacteria::nodes::Nop>(pIf->condition->location);
                }
#undef WHEN_TYPE_IS
                auto body_ty = lctx->get_type(pIf->body.get());
                Type *else_ty = nullptr;
                auto true_ty = body_ty;
                if (pIf->els.has_value()) {
                    else_ty = lctx->get_type(pIf->els.value().get());
                    // Now we have to get a peer type between the two for the true type
                    true_ty = peer_type({body_ty, else_ty}, gc);
                }
                auto body_lctx = gc.gcnew<LocalContext>(lctx, true_ty);
                body_ty = body_lctx->get_type(pIf->body.get());
                if (else_ty != nullptr) {
                    else_ty = body_lctx->get_type(pIf->els.value().get());
                }
                // Now we will have to do casting which will be fine
                // But gods we are going to have so many temporary variables that will get garbage collected in the final version of this, which is *fine*
                bacteria::BacteriaPtr body_ptr;
                if (dynamic_cast<VoidType *>(true_ty) != nullptr) {

                    auto compare = body_ty->compare(true_ty);
                    if (compare == -1) {
                        // We must throw an error here
                        cctx->globalContext->raise("Invalid cast", pIf->body->location,
                                                   error::ErrorCode::InvalidCast);
                    }
                    body_ptr = std::make_unique<bacteria::nodes::CastNode>(pIf->body->location,
                                                                           translate_expression(body_lctx, pIf->body),
                                                                           true_ty->get_cached_type());
                } else {
                    body_ptr = translate_expression(body_lctx, pIf->body);
                }
                if (pIf->els.has_value()) {
                    auto els = pIf->els.value();
                    bacteria::BacteriaPtr else_ptr;
                    if (dynamic_cast<VoidType *>(true_ty) != nullptr) {
                        auto compare = else_ty->compare(true_ty);
                        if (compare == -1) {
                            // We must throw an error here
                            cctx->globalContext->raise("Invalid cast", els->location,
                                                       error::ErrorCode::InvalidCast);
                        }
                        // Now we generate the cast, cast operators aren't a thing *yet* so we can do this easily
                        else_ptr = std::make_unique<bacteria::nodes::CastNode>(pIf->body->location,
                                                                               translate_expression(body_lctx, els),
                                                                               true_ty->get_cached_type());
                    } else {
                        else_ptr = translate_expression(body_lctx, els);
                    }
                    return std::make_unique<bacteria::nodes::If>(pIf->location, std::move(condition_ptr),
                                                                 std::move(body_ptr), std::move(else_ptr));
                } else {
                    return std::make_unique<bacteria::nodes::If>(pIf->location, std::move(condition_ptr),
                                                                 std::move(body_ptr));
                }
            }
        }
    }

    bacteria::BacteriaPtr translate_expression(LocalContext *lctx, parser::NodePtr expr) {
#define WHEN_EXPR_IS(type, name) if (auto name = dynamic_cast<type*>(true_expr); name)
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto true_expr = expr.get();
        WHEN_EXPR_IS(parser::nodes::TupleCall, pTupleCall) {
            return translate_tuple_call(lctx, pTupleCall);
        }
        WHEN_EXPR_IS(parser::nodes::If, pIf) {
            return translate_if_statement(lctx, pIf);
        }
        WHEN_EXPR_IS(parser::nodes::ValueReference, pValueReference) {
            if (auto comptime_attempt = cctx->get(pValueReference->name); comptime_attempt.has_value()) {
                auto comptime = comptime_attempt.value().get();
                NOT_IMPL_FOR("Comptime value references");
            } else if (auto runtime_attempt = rctx->get(pValueReference->name); runtime_attempt.has_value()) {
                auto runtime = runtime_attempt.value();
                return std::make_unique<bacteria::nodes::ValueReference>(pValueReference->location,
                                                                         runtime.runtime_name);
            }
        }
        WHEN_EXPR_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
            return std::make_unique<bacteria::nodes::IntegerLiteral>(pIntegerLiteral->location, pIntegerLiteral->value,
                                                                     lctx->expected_type
                                                                     ? lctx->expected_type->get_cached_type()
                                                                     : IntegerType::get(gc, true,
                                                                                        64)->get_cached_type());
        }
        WHEN_EXPR_IS(parser::nodes::EqualTo, pEqualTo) {
            auto bin_type = lctx->get_binary_type(pEqualTo->lhs.get(), pEqualTo->rhs.get());
            auto peer = peer_type({bin_type.first, bin_type.second}, gc);
            auto peer_ctx = gc.gcnew<LocalContext>(lctx, peer);
            // At some point we are going to need to respect operators which will be fun
            auto lhs_expr = translate_expression(peer_ctx, pEqualTo->lhs);
            auto rhs_expr = translate_expression(peer_ctx, pEqualTo->rhs);
            return std::make_unique<bacteria::nodes::EqualToNode>(pEqualTo->location, std::move(lhs_expr),
                                                                  std::move(rhs_expr));
        }
        WHEN_EXPR_IS(parser::nodes::Multiplication, pMultiplication) {
            auto bin_type = lctx->get_binary_type(pMultiplication->lhs.get(), pMultiplication->rhs.get());
            auto peer = peer_type({bin_type.first, bin_type.second}, gc);
            auto peer_ctx = gc.gcnew<LocalContext>(lctx, peer);
            // At some point we are going to need to respect operators which will be fun
            auto lhs_expr = translate_expression(peer_ctx, pMultiplication->lhs);
            auto rhs_expr = translate_expression(peer_ctx, pMultiplication->rhs);
            return std::make_unique<bacteria::nodes::MultiplyNode>(pMultiplication->location, std::move(lhs_expr),
                                                                   std::move(rhs_expr));
        }
        WHEN_EXPR_IS(parser::nodes::Subtraction, pSubtraction) {
            auto bin_type = lctx->get_binary_type(pSubtraction->lhs.get(), pSubtraction->rhs.get());
            auto peer = peer_type({bin_type.first, bin_type.second}, gc);
            auto peer_ctx = gc.gcnew<LocalContext>(lctx, peer);
            // At some point we are going to need to respect operators which will be fun
            auto lhs_expr = translate_expression(peer_ctx, pSubtraction->lhs);
            auto rhs_expr = translate_expression(peer_ctx, pSubtraction->rhs);
            return std::make_unique<bacteria::nodes::SubtractNode>(pSubtraction->location, std::move(lhs_expr),
                                                                   std::move(rhs_expr));
        }
#undef WHEN_EXPR_IS
        NOT_IMPL_FOR(typeid(*true_expr).name());
    }

    // Translate statement needs only a runtime context as there is no "expected type"
    void translate_statement(RuntimeContext *rctx, parser::NodePtr stmnt) {
        auto true_statement = stmnt.get();

        NOT_IMPL_FOR(typeid(*true_statement).name());
    }

    void translate_block(RuntimeContext *rctx, parser::nodes::Block *block) {
        for (const auto &i: block->children) {
            translate_statement(rctx, i);
        }
    }

    void translate_function_body(RuntimeContext *rctx, parser::NodePtr functionBody) {
        auto true_body = functionBody.get();
        if (auto as_block = dynamic_cast<parser::nodes::Block *>(true_body); as_block) {
            // We just translate a block!
            translate_block(rctx, as_block);
        } else {
            // We translate this as a return instruction + what ever is on the other side!
            auto lctx = rctx->comptime->globalContext->gc.gcnew<LocalContext>(rctx, rctx->functionReturnType);
            rctx->local_reciever->recieve((new bacteria::nodes::Return(functionBody->location,
                                                                       translate_expression(lctx,
                                                                                            functionBody)))->get());
        }
    }


}