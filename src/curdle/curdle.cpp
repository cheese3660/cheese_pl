//
// Created by Lexi Allen on 3/20/2023.
//
#include <utility>

#include "curdle/curdle.h"

#include "project/GlobalContext.h"

#include "parser/parser.h"

#include "parser/nodes/other_nodes.h"
#include "parser/nodes/single_member_nodes.h"
#include "parser/nodes/terminal_nodes.h"
#include "curdle/comptime.h"
#include "curdle/Interface.h"

#include "error.h"

#include "bacteria/BacteriaNode.h"
#include "bacteria/nodes/expression_nodes.h"
#include "curdle/builtin.h"
#include "curdle/values/ComptimeInteger.h"
#include "curdle/values/ComptimeType.h"
#include "curdle/types/TypeType.h"
#include "curdle/types/IntegerType.h"
#include "curdle/types/BooleanType.h"
#include "curdle/types/ReferenceType.h"
#include "curdle/values/ComptimeFunctionSet.h"
#include "curdle/values/ComptimeFloat.h"
#include "curdle/types/VoidType.h"
#include "curdle/types/NoReturnType.h"
#include "curdle/values/ComptimeArray.h"
#include "curdle/values/ComptimeObject.h"
#include "curdle/types/Float64Type.h"
#include "curdle/values/ComptimeComplex.h"

using namespace cheese::memory::garbage_collection;

namespace cheese::curdle {


    void resolve_mixin(ComptimeContext *ctx, parser::nodes::Mixin *mixin) {

    }

    // Likely going to need more parameters, but this should be fine
    gcref<Structure>
    translate_structure(ComptimeContext *ctx, parser::nodes::Structure *structure_node) {
        auto &gc = ctx->globalContext->gc;
        auto &global = ctx->globalContext;
        auto structure_ref = gc.gcnew<Structure>(ctx->get_structure_name(), ctx, gc);
        auto localCtx = structure_ref->containedContext;
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
                    auto temp = gc.gcnew<FunctionTemplate>(localCtx, child);
                    if (!structure_ref->function_sets.contains(as_op->op)) {
                        structure_ref->function_sets[as_op->op] = gc.gcnew<FunctionSet>();
                    }
                    structure_ref->function_sets[as_op->op]->templates.push_back(temp.get());
                } else if (auto as_fn = dynamic_cast<parser::nodes::Function *>(ptr); as_fn) {
                    auto temp = gc.gcnew<FunctionTemplate>(localCtx, child);
                    auto tptr = temp.get();
                    if (as_fn->flags.entry) ctx->globalContext->entry_function = tptr;
                    if (!structure_ref->function_sets.contains(as_fn->name)) {
                        structure_ref->function_sets[as_fn->name] = gc.gcnew<FunctionSet>();
                    }
                    structure_ref->function_sets[as_fn->name]->templates.push_back(tptr);
                } else if (auto as_gen = dynamic_cast<parser::nodes::Generator *>(ptr); as_gen) {
                    auto temp = gc.gcnew<FunctionTemplate>(localCtx, child);
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

    void setup_root_structure(cheese::project::GlobalContext *ctx, const parser::NodePtr &ast_node) {
        garbage_collector &gc = ctx->gc;
        auto newContext = gc.gcnew<ComptimeContext>(ctx, ctx->project.root_path, ctx->project.folder);
        auto translated = translate_structure(newContext,
                                              dynamic_cast<parser::nodes::Structure *>(ast_node.get()));
        ctx->root_structure = translated;
        newContext->pop_structure_name();
    }


    std::unique_ptr<bacteria::BacteriaNode> curdle(cheese::project::GlobalContext *gctx) {
        // First off create a structure for the main project file
        setup_root_structure(gctx, gctx->project.root_file);
        if (gctx->project.type == ProjectType::Application) {
            // We do a depth first search to find the entry point
            gctx->root_structure->search_entry();
            if (gctx->entry_function == nullptr) {
                error::raise_exiting_error("curdle", "no entrypoint found", gctx->project.root_file->location,
                                           error::ErrorCode::NoEntryPoint);
            }
            gctx->entry_function->get({});
        }
        return gctx->global_receiver.release()->get();
    }

    bacteria::BacteriaPtr translate_expression(LocalContext *lctx, parser::NodePtr expr);

    // This will handle automatic referencing and such
    bacteria::BacteriaPtr make_cast(LocalContext *lctx, Coordinate location, bacteria::BacteriaPtr castee, Type *from) {
        if (lctx->expected_type->compare(from) == 0) return castee;
        if (auto as_reference = dynamic_cast<ReferenceType *>(lctx->expected_type); as_reference) {
            return std::make_unique<bacteria::nodes::ImplicitReferenceNode>(location, make_cast(
                    lctx->runtime->comptime->globalContext->gc.gcnew<LocalContext>(lctx, as_reference->child),
                    location,
                    std::move(castee), from));
        }
        return std::make_unique<bacteria::nodes::CastNode>(location, std::move(castee),
                                                           lctx->expected_type->get_cached_type());
    }

    bacteria::BacteriaPtr make_cast(LocalContext *lctx, parser::NodePtr ptr) {
        return make_cast(lctx, ptr->location, translate_expression(lctx, ptr), lctx->get_type(ptr.get()));
    }


    bacteria::BacteriaPtr generate_call(LocalContext *lctx, const ConcreteFunction *function,
                                        const parser::NodeList &arguments, const Coordinate &location) {
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        std::vector<bacteria::BacteriaPtr> actual_arguments;
        for (int i = 0; i < arguments.size(); i++) {
            if (function->arguments[i].is_type) {
                auto &farg = function->arguments[i];
                auto parg = arguments[i];
                auto fctx = gc.gcnew<LocalContext>(rctx, farg.type);
                auto pty = fctx->get_type(parg.get());
                auto compare = farg.type->compare(pty);
                if (compare == 0) {
                    actual_arguments.push_back(translate_expression(fctx, parg));
                } else {
                    //TODO: more complex casting code once we add cast operators
                    // Now we do the check if I need to do a possible rvalue reference;

                    actual_arguments.push_back(
                            make_cast(fctx, parg->location, translate_expression(fctx, parg), pty));
                }
            }
        }
        return std::make_unique<bacteria::nodes::NormalCallNode>(location, function->mangled_name,
                                                                 std::move(actual_arguments));
    }

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
                auto function = get_function(rctx, cctx, lctx, function_set->set, call->args);
                auto arguments = call->args;
                auto location = call->location;
                return generate_call(lctx, function, arguments, location);
            }
            NOT_IMPL_FOR(typeid(*ctime).name());
        } else if (auto subscript = dynamic_cast<parser::nodes::Subscription *>(call->object.get()); subscript) {
            auto subscript_type = rctx->get_type(subscript->lhs.get());
            auto function = dynamic_cast<parser::nodes::ValueReference *>(subscript->rhs.get());
            if (!function) {
                NOT_IMPL_FOR("Possible function pointers");
            }
            auto structure = dynamic_cast<Structure *>(subscript_type.get());
            if (!structure) {
                auto ref = dynamic_cast<ReferenceType *>(subscript_type.get());
                if (!(structure = dynamic_cast<Structure *>(ref->child))) {
                    NOT_IMPL_FOR("Non structure subscriptions");
                }
            }
            // We are going to have to check interfaces and mixins at some point which will be fun, also function pointers
            if (!structure->function_sets.contains(function->name));
            parser::NodeList arg_nodes{};
            arg_nodes.push_back(subscript->lhs);
            for (auto &arg: call->args) {
                arg_nodes.push_back(arg);
            }
            ConcreteFunction *function2 = get_function(rctx, cctx, empty_ctx, structure->function_sets[function->name],
                                                       arg_nodes);
            return generate_call(lctx, function2, arg_nodes, call->location);
        } else {
            // Here is where we need to grab a function pointer and call the function pointer
            NOT_IMPL_FOR("non compile time deductible functions");
        }
    }

    bacteria::BacteriaPtr translate_object_call(LocalContext *lctx, parser::nodes::ObjectCall *call) {
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto empty_ctx = gc.gcnew<LocalContext>(rctx);
        if (auto attempt = cctx->try_exec(call->object.get(), rctx); attempt.has_value()) {
            auto ctime = attempt->get();
#define WHEN_CTIME_IS(type, name) if (auto name = dynamic_cast<type*>(ctime); name)
            WHEN_CTIME_IS(ComptimeFunctionSet, pComptimeFunctionSet) {
                // Essentially this is a wrapper that creates an anonymous object that calls the function with it :3
                // That or it has to look at the function declaration to match variables with names in the second case, which is less fun
                // Wait this is how we can do variable argument functions, a second overload of tuple call that creates a tuple out of values but that sounds like *absolute* hell
                NOT_IMPL_FOR("Functions");
            }
            WHEN_CTIME_IS(ComptimeType, pComptimeType) {
                auto tvalue = pComptimeType->typeValue;
#define WHEN_TYPE_IS(type, name) if (auto name = dynamic_cast<type*>(tvalue); name)
                WHEN_TYPE_IS(Structure, pStructure) {
                    if (pStructure->function_sets.contains("{}")) {
                        //todo: check if it contains one w/o a self argument
                        NOT_IMPL_FOR("Constructors");
                    } else {
                        auto type = pStructure->get_cached_type();
                        std::vector<bacteria::BacteriaPtr> initialized_values;
                        initialized_values.reserve(pStructure->fields.size());
                        // We need to give an error if no fields are initialized, and also an error if they are initialized out of order
                        int next_check_index = 0;
                        auto get_field_index = [&](const std::string &name) -> int {
                            for (int i = 0; i < pStructure->fields.size(); i++) {
                                if (pStructure->fields[i].name == name) return i;
                            }
                            return -1;
                        };

                        for (auto &field: call->args) {
                            auto field_ptr = (parser::nodes::FieldLiteral *) (field.get());
                            auto idx = get_field_index(field_ptr->name);
                            if (idx == -1) {
                                gctx->raise(
                                        "Invalid Field: " + field_ptr->name + " is not a field of " + pStructure->name,
                                        field->location, error::ErrorCode::InvalidField);
                                return std::make_unique<bacteria::nodes::Nop>(call->location);
                            }
                            if (idx != next_check_index++) {
                                gctx->raise(
                                        "Wrong Order: Structure fields must be initialized in order of where they come in the structure",
                                        field->location, error::ErrorCode::OutOfOrderInitialization);
                                return std::make_unique<bacteria::nodes::Nop>(call->location);
                            }
                            auto ctx = gc.gcnew<LocalContext>(rctx, pStructure->fields[idx].type);
                            initialized_values.push_back(translate_expression(ctx, field_ptr->value));
                        }
                        if (next_check_index != initialized_values.size()) {
                            gctx->raise(
                                    "Incomplete Initialization: all fields of a structure must be initialized",
                                    call->location, error::ErrorCode::IncompleteInitialization
                            );
                            return std::make_unique<bacteria::nodes::Nop>(call->location);
                        }
                        return std::make_unique<bacteria::nodes::AggregrateObject>(call->location, type,
                                                                                   std::move(initialized_values));
                    }
                }
#undef WHEN_TYPE_IS
                NOT_IMPL_FOR(tvalue->to_string());
            }
#undef WHEN_CTIME_IS


            NOT_IMPL_FOR(ctime->type->to_string());
        } else {
            NOT_IMPL_FOR("non compile time deductible objects");
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
                auto ty = empty_ctx->get_type(pIf->condition.get()).get();
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
                auto true_ty = body_ty.get();
                if (pIf->els.has_value()) {
                    else_ty = lctx->get_type(pIf->els.value().get());
                    // Now we have to get a peer type between the two for the true type
                    true_ty = peer_type({body_ty, else_ty}, gctx);
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
                    body_ptr = make_cast(body_lctx, pIf->body->location, translate_expression(body_lctx, pIf->body),
                                         body_ty);
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
                        body_ptr = make_cast(body_lctx, els->location, translate_expression(body_lctx, els),
                                             else_ty);
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

    bool trivial_read_node(parser::Node *node) {
#define TRIVIAL_FOR(type) if (dynamic_cast<type*>(node)) return true
        TRIVIAL_FOR(parser::nodes::ValueReference);
#undef TRIVIAL_FOR
        return false;
    }

    // Match statements are going to be so much more complex once I add actual destructuring to them and such

    bool single_catch_all(parser::nodes::MatchArm *node) {
        return (node->matches.size() == 1 && dynamic_cast<parser::nodes::MatchAll *>(node->matches[0].get()));
    }

    bacteria::BacteriaPtr
    make_condition(LocalContext *lctx, Type *value_type, parser::NodePtr value_gen, parser::Node *match) {
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
#define WHEN_MATCH_IS(type, name) if (auto name = dynamic_cast<type*>(match); name)
        WHEN_MATCH_IS(parser::nodes::MatchValue, pMatchValue) {
            auto match_against_context = gc.gcnew<LocalContext>(rctx, value_type);
            // This might be need to be redone for enum constants in the future
            auto rhs = make_cast(match_against_context, pMatchValue->value);
            auto lhs = translate_expression(match_against_context, std::move(value_gen));
            return std::make_unique<bacteria::nodes::EqualToNode>(match->location, std::move(lhs), std::move(rhs));
        }
#undef WHEN_MATCH_IS
        NOT_IMPL_FOR(typeid(*match).name());
    }

    bacteria::BacteriaPtr translate_match_statement(LocalContext *lctx, parser::nodes::Match *pMatch) {
        bool trivial = trivial_read_node(pMatch->value.get());
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto peer_type = lctx->get_type(pMatch);
        auto peer_ctx = gc.gcnew<LocalContext>(rctx, peer_type);
        auto value_type = rctx->get_type(pMatch->value.get());
        if (pMatch->arms.size() == 0) {
            gctx->raise("Invalid Match Statement: A match statement must have at least one arm", pMatch->location,
                        error::ErrorCode::InvalidMatchStatement);
            return std::make_unique<bacteria::nodes::Nop>(pMatch->location);
        }
        std::string possible_name = "";
        if (!trivial) {
            NOT_IMPL_FOR("Non trivial copy match statements");
        }
        // We have to start from the top going to the bottom which is going to be pain
        bacteria::nodes::If *parent = nullptr;
        bacteria::BacteriaPtr top_level_if;
        for (int i = 0; i < pMatch->arms.size(); i++) {
            auto arm = (parser::nodes::MatchArm *) pMatch->arms[i].get();
            // Check if the arm is a single catchall node
            bool first = i == 0;
            if (single_catch_all(arm)) {
                // We are going to ignore any captures for now as setting up the if structure for captures will be actual pain
                // As it's going to involve a completely different way of setting up an arm in the lower level
                auto ptr = make_cast(peer_ctx, arm->body->location, translate_expression(peer_ctx, arm->body),
                                     peer_ctx->get_type(arm->body.get()));
                if (first) {
                    top_level_if = std::move(ptr);
                } else {
                    parent->els = std::move(ptr);
                }
                if (i != pMatch->arms.size() - 1) {
                    gctx->raise("Invalid Match Statement: The catchall statement must be the last statement",
                                arm->location, error::ErrorCode::InvalidMatchStatement);
                    return std::make_unique<bacteria::nodes::Nop>(pMatch->location);
                }
            } else {
                bacteria::BacteriaPtr base_condition;
                for (auto &match: arm->matches) {
                    auto condition = make_condition(lctx, value_type, pMatch->value, match.get());
                    if (base_condition) {
                        base_condition = std::make_unique<bacteria::nodes::OrNode>(arm->location,
                                                                                   std::move(base_condition),
                                                                                   std::move(condition));
                    } else {
                        base_condition = std::move(condition);
                    }
                }
                auto _if = std::make_unique<bacteria::nodes::If>(arm->location, std::move(base_condition),
                                                                 make_cast(peer_ctx, arm->body));
                if (parent) {
                    auto gotten = _if.get();
                    parent->els = std::move(_if);
                    parent = gotten;
                } else {
                    parent = _if.get();
                    top_level_if = std::move(_if);
                }
            }
        }

        return top_level_if;
    }

    template<typename T>
    bacteria::BacteriaPtr
    translate_binary(LocalContext *lctx, Coordinate location, parser::NodePtr lhs, parser::NodePtr rhs) {
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        static_assert(std::is_base_of_v<bacteria::nodes::BinaryNode, T>, "Class must be of type binary node");
        auto bin_type = lctx->get_binary_type(lhs.get(), rhs.get());
        auto peer = peer_type({bin_type.first, bin_type.second}, gctx);
        auto peer_ctx = gc.gcnew<LocalContext>(lctx, peer);
        // At some point we are going to need to respect operators which will be fun
        auto lhs_expr = make_cast(peer_ctx, lhs);
        auto rhs_expr = make_cast(peer_ctx, rhs);
        return std::make_unique<T>(location, std::move(lhs_expr),
                                   std::move(rhs_expr));
    }


    bacteria::BacteriaPtr translate_comptime(LocalContext *lctx, Coordinate location, ComptimeValue *value) {
        auto &gc = lctx->runtime->comptime->globalContext->gc;
        auto result_type = lctx->expected_type ? lctx->expected_type : value->type;
        if (result_type->get_comptimeness() == Comptimeness::Comptime) {
            throw CurdleError(
                    "Not Runtime: cannot convert value of type " + value->type->to_string() + " to a runtime literal",
                    error::ErrorCode::NotRuntime);
        }
        gcref<ComptimeValue> v = {lctx->runtime->comptime->globalContext->gc, nullptr};
        if (value->type->compare(result_type) != 0) {
            v = std::move(value->cast(result_type, lctx->runtime->comptime->globalContext->gc));
        } else {
            v = {lctx->runtime->comptime->globalContext->gc, value};
        }
        auto vv = v.get();
#define WHEN_COMPTIME_IS(type, name) if (auto name = dynamic_cast<type*>(vv); name)
        WHEN_COMPTIME_IS(ComptimeInteger, pComptimeInteger) {
            return std::make_unique<bacteria::nodes::IntegerLiteral>(location, pComptimeInteger->value,
                                                                     result_type->get_cached_type());
        }
        WHEN_COMPTIME_IS(ComptimeFloat, pComptimeFloat) {
            return std::make_unique<bacteria::nodes::FloatLiteral>(location, pComptimeFloat->value,
                                                                   result_type->get_cached_type());
        }
        WHEN_COMPTIME_IS(ComptimeArray, pComptimeArray) {
            if (auto as_structure = dynamic_cast<Structure *>(pComptimeArray->type); as_structure) {
                auto t = as_structure->get_cached_type();
                std::vector<bacteria::BacteriaPtr> values;
                for (int i = 0; i < pComptimeArray->values.size(); i++) {
                    auto field = pComptimeArray->values[i];
                    auto lower_context = gc.gcnew<LocalContext>(lctx, as_structure->fields[i].type);
                    values.push_back(translate_comptime(lower_context, location, field));
                }
                return std::make_unique<bacteria::nodes::AggregrateObject>(location, t, std::move(values));
            }
        }
        WHEN_COMPTIME_IS(ComptimeObject, pComptimeObject) {
            if (auto as_structure = dynamic_cast<Structure *>(pComptimeObject->type); as_structure) {
                auto t = as_structure->get_cached_type();
                std::vector<bacteria::BacteriaPtr> values;
                for (auto &field: as_structure->fields) {
                    auto lower_context = gc.gcnew<LocalContext>(lctx, field.type);
                    values.push_back(translate_comptime(lower_context, location, pComptimeObject->fields[field.name]));
                }
                return std::make_unique<bacteria::nodes::AggregrateObject>(location, t, std::move(values));
            }
        }
        WHEN_COMPTIME_IS(ComptimeComplex, pComptimeComplex) {
            return std::make_unique<bacteria::nodes::ComplexLiteral>(location, pComptimeComplex->a, pComptimeComplex->b,
                                                                     result_type->get_cached_type());
        }

#undef WHEN_COMPTIME_IS
        NOT_IMPL_FOR(typeid(*vv).name());
    }

    bacteria::BacteriaPtr translate_expression(LocalContext *lctx, parser::NodePtr expr) {
        auto rctx = lctx->runtime;
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto true_expr = expr.get();
        try {
            auto execed = cctx->try_exec(expr.get(), rctx);
            if (execed.has_value()) {
                return translate_comptime(lctx, expr->location, execed.value().get());
            }

#define WHEN_EXPR_IS(type, name) if (auto name = dynamic_cast<type*>(true_expr); name)
#define NOP() return std::make_unique<bacteria::nodes::Nop>(expr->location)
            WHEN_EXPR_IS(parser::nodes::TupleCall, pTupleCall) {
                return translate_tuple_call(lctx, pTupleCall);
            }
            WHEN_EXPR_IS(parser::nodes::If, pIf) {
                return translate_if_statement(lctx, pIf);
            }
            WHEN_EXPR_IS(parser::nodes::Match, pMatch) {
                return translate_match_statement(lctx, pMatch);
            }
            WHEN_EXPR_IS(parser::nodes::ValueReference, pValueReference) {
                if (auto comptime_attempt = cctx->get(pValueReference->name); comptime_attempt.has_value()) {
                    auto comptime = comptime_attempt.value().get();
                    NOT_IMPL_FOR("Comptime value references");
                } else if (auto runtime_attempt = rctx->get(pValueReference->name); runtime_attempt.has_value()) {
                    auto runtime = runtime_attempt.value();
                    return std::make_unique<bacteria::nodes::ValueReference>(pValueReference->location,
                                                                             runtime.runtime_name);
                } else {
                    gctx->raise("Invalid Variable Reference: " + pValueReference->name +
                                " does not exist in the current scope", pValueReference->location,
                                error::ErrorCode::InvalidVariableReference);
                }
            }
            WHEN_EXPR_IS(parser::nodes::Self, pSelf) {
                if (auto runtime_attempt = rctx->get("self"); runtime_attempt.has_value()) {
                    auto runtime = runtime_attempt.value();
                    return std::make_unique<bacteria::nodes::ValueReference>(pSelf->location,
                                                                             runtime.runtime_name);
                } else {
                    gctx->raise("Invalid Variable Reference: self does not exist in the current scope", pSelf->location,
                                error::ErrorCode::InvalidVariableReference);
                }
            }
            WHEN_EXPR_IS(parser::nodes::IntegerLiteral, pIntegerLiteral) {
                return std::make_unique<bacteria::nodes::IntegerLiteral>(pIntegerLiteral->location,
                                                                         pIntegerLiteral->value,
                                                                         lctx->expected_type
                                                                         ? lctx->expected_type->get_cached_type()
                                                                         : IntegerType::get(gctx, true,
                                                                                            64)->get_cached_type());
            }
            WHEN_EXPR_IS(parser::nodes::EqualTo, pEqualTo) {
                return translate_binary<bacteria::nodes::EqualToNode>(lctx, expr->location, pEqualTo->lhs,
                                                                      pEqualTo->rhs);
            }
            WHEN_EXPR_IS(parser::nodes::LesserThan, pLesserThan) {
                return translate_binary<bacteria::nodes::LesserThanNode>(lctx, expr->location, pLesserThan->lhs,
                                                                         pLesserThan->rhs);
            }
            WHEN_EXPR_IS(parser::nodes::Multiplication, pMultiplication) {
                return translate_binary<bacteria::nodes::MultiplyNode>(lctx, expr->location, pMultiplication->lhs,
                                                                       pMultiplication->rhs);
            }
            WHEN_EXPR_IS(parser::nodes::Subtraction, pSubtraction) {
                return translate_binary<bacteria::nodes::SubtractNode>(lctx, expr->location, pSubtraction->lhs,
                                                                       pSubtraction->rhs);
            }
            WHEN_EXPR_IS(parser::nodes::Modulus, pModulus) {
                return translate_binary<bacteria::nodes::ModulusNode>(lctx, expr->location, pModulus->lhs,
                                                                      pModulus->rhs);
            }
            WHEN_EXPR_IS(parser::nodes::Division, pDivision) {
                return translate_binary<bacteria::nodes::DivisionNode>(lctx, expr->location, pDivision->lhs,
                                                                       pDivision->rhs);
            }
            WHEN_EXPR_IS(parser::nodes::Addition, pAddition) {
                return translate_binary<bacteria::nodes::AdditionNode>(lctx, expr->location, pAddition->lhs,
                                                                       pAddition->rhs);
            }
            WHEN_EXPR_IS(parser::nodes::Cast, pCast) {
                try {
                    auto rhs = cctx->exec(pCast->rhs, rctx);
                    // We aren't going to make sure things are semantically correct *just* yet
                    if (auto as_type = dynamic_cast<ComptimeType *>(rhs.get()); as_type) {
                        auto cast_context = gc.gcnew<LocalContext>(lctx, as_type->typeValue);
                        auto lhs_type = cast_context->get_type(pCast->lhs.get());
                        auto compare = as_type->typeValue->compare(lhs_type, false);
                        if (compare == 0) {
                            return translate_expression(cast_context, pCast->lhs);
                        } else if (compare > 0) {
                            return make_cast(cast_context, pCast->location, translate_expression(cast_context,
                                                                                                 pCast->lhs), lhs_type);
                        } else {
                            gctx->raise("Invalid Cast: Cannot cast " + lhs_type->to_string() + " to " +
                                        as_type->typeValue->to_string(), pCast->location,
                                        error::ErrorCode::InvalidCast);
                            NOP();
                        }
                    } else {
                        gctx->raise("Invalid Cast: Casts must be to types, not: " + rhs->type->to_string(),
                                    pCast->rhs->location, error::ErrorCode::InvalidCast);
                        NOP();
                    }
                } catch (const NotComptimeError &e) {
                    gctx->raise("Invalid Cast: Attempting to cast to a non comptime known type", pCast->rhs->location,
                                error::ErrorCode::InvalidCast);
                    NOP();
                }
            }
            WHEN_EXPR_IS(parser::nodes::Return, pReturn) {
                auto target_type = lctx->runtime->functionReturnType;
                if (dynamic_cast<NoReturnType *>(target_type)) {
                    gctx->raise("Unexpected Return: Return expression found in function marked noreturn",
                                pReturn->location,
                                error::ErrorCode::UnexpectedReturn);
                    NOP();
                }
                if (dynamic_cast<VoidType *>(target_type)) {
                    gctx->raise(
                            "Unexpected Return: Found value returning return expression in function marked with the return type 'void'",
                            pReturn->location, error::ErrorCode::UnexpectedReturn);
                    NOP();
                }
                auto tctx = gc.gcnew<LocalContext>(rctx, target_type);
                auto child_type = tctx->get_type(pReturn->child.get());
                auto compare = target_type->compare(child_type);
                std::cout << compare << '\n';
                if (compare == 0) {
                    return std::make_unique<bacteria::nodes::Return>(pReturn->location,
                                                                     translate_expression(tctx, pReturn->child));
                } else if (compare > 0) {
                    return std::make_unique<bacteria::nodes::Return>(pReturn->location,
                                                                     make_cast(tctx, pReturn->location,
                                                                               translate_expression(tctx,
                                                                                                    pReturn->child),
                                                                               child_type));
                } else {
                    gctx->raise("Invalid Cast: Attempting to cast value of type " + child_type->to_string() +
                                " to a value of type " + target_type->to_string() + " implicitly in return statement",
                                pReturn->child->location, error::ErrorCode::InvalidCast);
                    NOP();
                }
            }
            WHEN_EXPR_IS(parser::nodes::UnaryMinus, pUnaryMinus) {
                return std::make_unique<bacteria::nodes::UnaryMinusNode>(pUnaryMinus->location,
                                                                         translate_expression(lctx,
                                                                                              pUnaryMinus->child));
            }
            WHEN_EXPR_IS(parser::nodes::UnaryPlus, pUnaryPlus) {
                return std::make_unique<bacteria::nodes::UnaryPlusNode>(pUnaryPlus->location,
                                                                        translate_expression(lctx,
                                                                                             pUnaryPlus->child));
            }
            WHEN_EXPR_IS(parser::nodes::ObjectCall, pObjectCall) {
                return translate_object_call(lctx, pObjectCall);
            }
            WHEN_EXPR_IS(parser::nodes::FloatLiteral, pFloatLiteral) {
                return std::make_unique<bacteria::nodes::FloatLiteral>(pFloatLiteral->location, pFloatLiteral->value,
                                                                       lctx->expected_type
                                                                       ? lctx->expected_type->get_cached_type()
                                                                       : Float64Type::get(gctx)->get_cached_type());
            }
            WHEN_EXPR_IS(parser::nodes::Subscription, pSubscription) {
                auto subscript_type = rctx->get_type(pSubscription->lhs.get());
                auto subscript_ctx = gc.gcnew<LocalContext>(rctx, subscript_type);
                auto identifier = dynamic_cast<parser::nodes::ValueReference *>(pSubscription->rhs.get());
                std::string name;
                auto structure = dynamic_cast<Structure *>(subscript_type.get());
                bool reference = false;
                if (!structure) {
                    reference = true;
                    auto ref = dynamic_cast<ReferenceType *>(subscript_type.get());
                    if (!ref) {
                        NOT_IMPL_FOR("Non structure subscriptions");
                    }
                    if (!(structure = dynamic_cast<Structure *>(ref->child))) {
                        NOT_IMPL_FOR("Non structure subscriptions");
                    }
                }
                // This could be much more simplified TODO
                if (!identifier) {
                    auto integer = dynamic_cast<parser::nodes::IntegerLiteral *>(pSubscription->rhs.get());
                    if (integer) {
                        if (structure->is_tuple) {
                            name = "_" + static_cast<std::string>(integer->value);
                        } else {
                            throw LocalizedCurdleError(
                                    "Invalid Subscript: Attempting to tuple index a non-tuple structure",
                                    pSubscription->location, error::ErrorCode::InvalidSubscript);
                        }
                    } else {
                        NOT_IMPL_FOR("Non identifier subscriptions");
                    }
                } else {
                    name = identifier->name;
                }
                for (int i = 0; i < structure->fields.size(); i++) {
                    auto &field = structure->fields[i];
                    if (field.name == name) {
                        // This is where we just return the operator to get a field
                        if (reference) {
                            return std::make_unique<bacteria::nodes::ReferenceSubscriptNode>(expr->location,
                                                                                             translate_expression(
                                                                                                     subscript_ctx,
                                                                                                     pSubscription->lhs),
                                                                                             i);
                        } else {
                            return std::make_unique<bacteria::nodes::ObjectSubscriptNode>(expr->location,
                                                                                          translate_expression(
                                                                                                  subscript_ctx,
                                                                                                  pSubscription->lhs),
                                                                                          i);
                        }
                    }
                }
                NOT_IMPL_FOR("Possible traits");
            }
            WHEN_EXPR_IS(parser::nodes::TupleLiteral, pTupleLiteral) {
                gcref<Type> expected_type =
                        lctx->expected_type && dynamic_cast<Structure *>(lctx->expected_type) ? gcref<Type>(gc,
                                                                                                            lctx->expected_type)
                                                                                              : rctx->get_type(
                                pTupleLiteral);
                std::vector<bacteria::BacteriaPtr> children;
                auto as_struct = dynamic_cast<Structure *>(expected_type.get());
                if (!as_struct || !as_struct->is_tuple) {
                    NOT_IMPL_FOR("Non tuple aggregrates");
                }
                if (as_struct->fields.size() != pTupleLiteral->children.size()) {
                    gctx->raise("Invalid Tuple Literal: Expected " + std::to_string(as_struct->fields.size()) +
                                " values, got " + std::to_string(pTupleLiteral->children.size()) + " values",
                                pTupleLiteral->location, error::ErrorCode::InvalidTupleLiteral);
                    NOP();
                }
                for (int i = 0; i < as_struct->fields.size(); i++) {
                    auto field = as_struct->fields[i];
                    auto ty = field.type;
                    auto ctx = gc.gcnew<LocalContext>(rctx, ty);
                    auto val = pTupleLiteral->children[i];
                    children.push_back(make_cast(ctx, val));
                }
                return std::make_unique<bacteria::nodes::AggregrateObject>(pTupleLiteral->location,
                                                                           expected_type->get_cached_type(),
                                                                           std::move(children));
            }
            WHEN_EXPR_IS(parser::nodes::ObjectLiteral, pObjectLiteral) {
                gcref<Type> expected_type =
                        lctx->expected_type && dynamic_cast<Structure *>(lctx->expected_type) ? gcref<Type>(gc,
                                                                                                            lctx->expected_type)
                                                                                              : rctx->get_type(
                                pObjectLiteral);
                std::vector<bacteria::BacteriaPtr> initialized_values;
                auto as_struct = dynamic_cast<Structure *>(expected_type.get());
                if (!as_struct) {
                    NOT_IMPL_FOR("Non struct aggregrates");
                }
                if (as_struct->fields.size() != pObjectLiteral->children.size()) {
                    gctx->raise("Invalid Object Literal: Expected " + std::to_string(as_struct->fields.size()) +
                                " values, got " + std::to_string(pObjectLiteral->children.size()) + " values",
                                pObjectLiteral->location, error::ErrorCode::InvalidObjectLiteral);
                    NOP();
                }
                int next_check_index = 0;
                auto get_field_index = [&](const std::string &name) -> int {
                    for (int i = 0; i < as_struct->fields.size(); i++) {
                        if (as_struct->fields[i].name == name) return i;
                    }
                    return -1;
                };

                for (auto &field: pObjectLiteral->children) {
                    auto field_ptr = (parser::nodes::FieldLiteral *) (field.get());
                    auto idx = get_field_index(field_ptr->name);
                    if (idx == -1) {
                        gctx->raise(
                                "Invalid Field: " + field_ptr->name + " is not a field of " + as_struct->name,
                                field->location, error::ErrorCode::InvalidField);
                        return std::make_unique<bacteria::nodes::Nop>(pObjectLiteral->location);
                    }
                    if (idx != next_check_index++) {
                        gctx->raise(
                                "Wrong Order: Structure fields must be initialized in order of where they come in the structure",
                                field->location, error::ErrorCode::OutOfOrderInitialization);
                        return std::make_unique<bacteria::nodes::Nop>(pObjectLiteral->location);
                    }
                    auto ctx = gc.gcnew<LocalContext>(rctx, as_struct->fields[idx].type);
                    initialized_values.push_back(translate_expression(ctx, field_ptr->value));
                }
                if (next_check_index != initialized_values.size()) {
                    gctx->raise(
                            "Incomplete Initialization: all fields of a structure must be initialized",
                            pObjectLiteral->location, error::ErrorCode::IncompleteInitialization
                    );
                    return std::make_unique<bacteria::nodes::Nop>(pObjectLiteral->location);
                }
                return std::make_unique<bacteria::nodes::AggregrateObject>(pObjectLiteral->location,
                                                                           as_struct->get_cached_type(),
                                                                           std::move(initialized_values));
            }
            WHEN_EXPR_IS(parser::nodes::AddressOf, pAddressOf) {
                //TODO: specific translation for lvalues, as lvalues are a thing
                return std::make_unique<bacteria::nodes::ReferenceNode>(pAddressOf->location, translate_expression(
                        gc.gcnew<LocalContext>(rctx), pAddressOf->child));
            }
#undef WHEN_EXPR_IS
            NOT_IMPL_FOR(typeid(*true_expr).name());
        } catch (const CurdleError &curdleError) {
            gctx->raise(curdleError.what(), expr->location, curdleError.code);
            NOP();
        } catch (const NotImplementedException &notImplementedException) {
            // Lets fall through not implemented exceptions to the user so we can see exactly where
            gctx->raise(notImplementedException.what(), expr->location, error::ErrorCode::GeneralCompilerError);
            NOP();
        } catch (const LocalizedCurdleError &localizedCurdleError) {
            gctx->raise(localizedCurdleError.what(), localizedCurdleError.location, localizedCurdleError.code);
            NOP();
        }
#undef NOP
    }

    void recieve_destructure(RuntimeContext *rctx, const parser::NodePtr &destructure, Type *destructure_type,
                             const parser::NodePtr &value_node);

    void recieve_tuple_destructure(RuntimeContext *rctx, parser::nodes::TupleDestructure *destructure,
                                   Type *destructure_type, const parser::NodePtr &value_node) {
        // Now we must assert that the destructure type is correct
        auto as_tuple = dynamic_cast<Structure *>(destructure_type);
        bool reference = false;
        if (!as_tuple) {
            reference = true;
            auto ref = dynamic_cast<ReferenceType *>(destructure_type);
            if (!ref) {
                throw LocalizedCurdleError("Invalid Destructure: Attempting to destructure a value of type " +
                                           destructure_type->to_string() + " w/ a tuple destructures",
                                           destructure->location, error::ErrorCode::InvalidDestructure);
            }
            if (!(as_tuple = dynamic_cast<Structure *>(ref->child))) {
                throw LocalizedCurdleError("Invalid Destructure: Attempting to destructure a value of type " +
                                           destructure_type->to_string() + " w/ a tuple destructures",
                                           destructure->location, error::ErrorCode::InvalidDestructure);
            }
        }
        if (!as_tuple->is_tuple) {
            throw LocalizedCurdleError("Invalid Destructure: Attempting to destructure a value of type " +
                                       destructure_type->to_string() + " w/ a tuple destructures",
                                       destructure->location, error::ErrorCode::InvalidDestructure);
        }
        if (destructure->children.size() > as_tuple->fields.size()) {
            throw LocalizedCurdleError("Invalid Destructure: Too many fields in tuple destructure, expected at most " +
                                       std::to_string(as_tuple->fields.size()),
                                       destructure->children[as_tuple->fields.size()]->location,
                                       error::ErrorCode::InvalidDestructure);
        }
        // At least with a tuple destructure you know everything is going to be in order
        for (int i = 0; i < destructure->children.size(); i++) {
            auto field = as_tuple->fields[i];
            auto match = destructure->children[i];
            auto new_node = (new parser::nodes::Subscription(
                    match->location,
                    value_node,
                    (new parser::nodes::IntegerLiteral(
                            match->location,
                            i))->get()))->get();
            if (auto as_def = dynamic_cast<parser::nodes::VariableDefinition *>(match.get()); as_def) {
                auto name = as_def->name;
                auto type = field.type;
                rctx->local_reciever->receive(
                        std::make_unique<bacteria::nodes::VariableInitializationNode>(match->location, name,
                                                                                      type->get_cached_type(),
                                                                                      make_cast(
                                                                                              rctx->comptime->globalContext->gc.gcnew<LocalContext>(
                                                                                                      rctx, type),
                                                                                              new_node)));
                rctx->variables[name] = RuntimeVariableInfo{as_def->flags.mut == 0, name, type};
            } else if (dynamic_cast<parser::nodes::Underscore *>(match.get())) {
                continue;
            } else {
                auto field_type = field.type;
                recieve_destructure(rctx, match, field_type, new_node);
            }
        }
    }

    void recieve_destructure(RuntimeContext *rctx, const parser::NodePtr &destructure, Type *destructure_type,
                             const parser::NodePtr &value_node) {
        auto true_destructure = destructure.get();
#define WHEN_DESTRUCTURE_IS(type, name) if (auto name = dynamic_cast<type*>(true_destructure); name)
        WHEN_DESTRUCTURE_IS(parser::nodes::TupleDestructure, pTupleDestructure) {
            recieve_tuple_destructure(rctx, pTupleDestructure, destructure_type, value_node);
            return;
        }
#undef WHEN_DESTRUCTURE_IS
        NOT_IMPL_FOR(typeid(*true_destructure).name());
    }

    // Translate statement needs only a runtime context as there is no "expected type"
    void translate_statement(RuntimeContext *rctx, parser::NodePtr stmnt) {
        auto cctx = rctx->comptime;
        auto gctx = cctx->globalContext;
        auto &gc = gctx->gc;
        auto true_statement = stmnt.get();
        try {
#define WHEN_STATEMENT_IS(type, name) if (auto name = dynamic_cast<type*>(true_statement); name)

            WHEN_STATEMENT_IS(parser::nodes::Assignment, pAssignment) {
                auto assignee = pAssignment->lhs;
                // Now we check if this is a discard statement as that is the most important at the moment.
                if (dynamic_cast<parser::nodes::Underscore *>(assignee.get())) {
                    rctx->local_reciever->receive(
                            translate_expression(rctx->comptime->globalContext->gc.gcnew<LocalContext>(rctx).get(),
                                                 pAssignment->rhs));
                    return;
                }
            }
            WHEN_STATEMENT_IS(parser::nodes::VariableDeclaration, pVariableDeclaration) {
                // Now we have to do variable declaration stuff :3
                auto definition = (parser::nodes::VariableDefinition *) pVariableDeclaration->def.get();
                if (definition->flags.comptime) {
                    // This is a compile time variable so we define it once in the compile time scope
                    try {
                        if (!definition->type.has_value()) {
                            cctx->comptimeVariables[definition->name] = gc.gcnew<ComptimeVariable>(
                                    rctx->get_type(pVariableDeclaration->value.get()),
                                    cctx->exec(pVariableDeclaration->value, rctx));
                        } else {
                            auto ty = cctx->exec(definition->type.value(), rctx);
                            if (auto as_type = dynamic_cast<ComptimeType *>(ty.get()); as_type) {
                                cctx->comptimeVariables[definition->name] = gc.gcnew<ComptimeVariable>(
                                        as_type->typeValue,
                                        cctx->exec(pVariableDeclaration->value, rctx)->cast(as_type->typeValue, gc));
                            } else {
                                gctx->raise("Expected a type: found " + ty->type->to_string(),
                                            definition->type.value()->location, error::ErrorCode::ExpectedType);
                            }
                        }
                    } catch (const NotComptimeError &e) {
                        gctx->raise(e.what(), pVariableDeclaration->location, error::ErrorCode::NotComptime);
                    } catch (const BadBuiltinCall &e) {
                        gctx->raise(e.what(), pVariableDeclaration->location, error::ErrorCode::BadBuiltinCall);
                    }
                    return;
                } else {
                    // Now we define the variable and assign it in a "single" breath
                    gcref<Type> result_type{gc, nullptr};
                    Type *value_type = nullptr; // This is the cached type
                    gcref<LocalContext> result_context{gc, nullptr};
                    if (definition->type.has_value()) {
                        try {
                            auto ty = cctx->exec(definition->type.value(), rctx);
                            if (auto as_type = dynamic_cast<ComptimeType *>(ty.get()); as_type) {
                                result_type = gc.manage(as_type->typeValue);
                                result_context = gc.gcnew<LocalContext>(rctx, result_type);
                                value_type = result_context->get_type(pVariableDeclaration->value.get());
                            }
                        } catch (const NotComptimeError &e) {
                            gctx->raise(e.what(), definition->location, error::ErrorCode::NotComptime);
                            return;
                        } catch (const BadBuiltinCall &e) {
                            gctx->raise(e.what(), definition->location, error::ErrorCode::BadBuiltinCall);
                            return;
                        }
                    } else {
                        result_type = gc.manage((value_type = rctx->get_type(pVariableDeclaration->value.get())));
                        result_context = gc.gcnew<LocalContext>(rctx, result_type);
                    }
                    auto compare = result_type->compare(value_type);
                    if (compare == -1) {
                        gctx->raise("Invalid Cast: Attempting to implicitly cast " + value_type->to_string() + " to " +
                                    result_type->to_string() +
                                    " in a variable declaration, which is an impossible cast",
                                    pVariableDeclaration->location, error::ErrorCode::InvalidCast);
                        return;
                    }
                    bacteria::BacteriaPtr inner_ptr = translate_expression(result_context, pVariableDeclaration->value);
                    if (compare > 0) {
                        inner_ptr = make_cast(result_context, pVariableDeclaration->location, std::move(inner_ptr),
                                              value_type);
                    }
                    rctx->local_reciever->receive(
                            std::make_unique<bacteria::nodes::VariableInitializationNode>(
                                    pVariableDeclaration->location,
                                    definition->name,
                                    result_type->get_cached_type(),
                                    std::move(inner_ptr)));
                    rctx->variables[definition->name] = RuntimeVariableInfo{!definition->flags.mut, definition->name,
                                                                            result_type};
                    return;
                }

            }
            WHEN_STATEMENT_IS(parser::nodes::Destructure, pDestructure) {
                auto destructure_type = rctx->get_type(pDestructure->value.get());
                // So now we quickly define a local variable
                parser::NodePtr reference;
                if (dynamic_cast<parser::nodes::ValueReference *>(pDestructure->value.get())) {
                    reference = pDestructure->value;
                } else {
                    auto var_name = gctx->get_anonymous_variable("destructure");
                    rctx->variables[var_name] = RuntimeVariableInfo{true, var_name, destructure_type};
                    rctx->local_reciever->receive(
                            std::make_unique<bacteria::nodes::VariableInitializationNode>(pDestructure->location,
                                                                                          var_name,
                                                                                          destructure_type->get_cached_type(),
                                                                                          translate_expression(
                                                                                                  gc.gcnew<LocalContext>(
                                                                                                          rctx,
                                                                                                          destructure_type),
                                                                                                  pDestructure->value))
                    );
                    reference = (new parser::nodes::ValueReference(pDestructure->value->location,
                                                                   var_name))->get();
                }
                // Lets do this only for structures now, we can do more later, but just as a proof of concept
                // Also more optimization will have to be done in the future, like checking if it's a bunch of literals and assigning them directly instead, instead of the intermediary, but that'll require a lot more context given to every function which is fine
                // We want to compile the program as *optimally* as possible before we pass it to LLVM or whatever backend
                // Setting up tests for this is going to be *painful*, but necessary before we go to the next level
                // But at least at the next level we can assert that test cases will work if we develop a good testing system for cheese
                // But we might need to set up more than just test cases running once we add optimization, as we want to make sure optimization always works
                recieve_destructure(rctx, pDestructure->structure, destructure_type,
                                    reference);
                return;
            }

#undef WHEN_STATEMENT_IS

            // Default implementation of a translate_statement for when its an expression
            auto ret_ty = rctx->get_type(stmnt.get());
            if (!(dynamic_cast<VoidType *>(ret_ty.get()) || dynamic_cast<NoReturnType *>(ret_ty.get()))) {
                rctx->comptime->globalContext->raise("Unused Value: Discarding value of type " + ret_ty->to_string() +
                                                     ", use '_ = ...' to explicitly disregard a value", stmnt->location,
                                                     error::ErrorCode::UnusedValue);
                return;
            }
            rctx->local_reciever->receive(
                    translate_expression(rctx->comptime->globalContext->gc.gcnew<LocalContext>(rctx).get(), stmnt));
//        NOT_IMPL_FOR(typeid(*true_statement).name());
        } catch (const CurdleError &curdleError) {
            gctx->raise(curdleError.what(), stmnt->location, curdleError.code);
        } catch (const NotImplementedException &notImplementedException) {
            // Lets fall through not implemented exceptions to the user so we can see exactly where
            gctx->raise(notImplementedException.what(), stmnt->location, error::ErrorCode::GeneralCompilerError);
        } catch (const LocalizedCurdleError &localizedCurdleError) {
            gctx->raise(localizedCurdleError.what(), localizedCurdleError.location, localizedCurdleError.code);
        }
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
            if (auto as_return = dynamic_cast<parser::nodes::Return *>(true_body); as_return) {
                rctx->local_reciever->receive((new bacteria::nodes::Return(functionBody->location,
                                                                           translate_expression(lctx,
                                                                                                as_return->child)))->get());
            } else {
                rctx->local_reciever->receive((new bacteria::nodes::Return(functionBody->location,
                                                                           translate_expression(lctx,
                                                                                                functionBody)))->get());
            }
        }
    }


}