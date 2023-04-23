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
}