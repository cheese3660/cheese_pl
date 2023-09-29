//
// Created by Lexi Allen on 3/30/2023.
//

#include "curdle/types/Structure.h"
#include "project/GlobalContext.h"
#include "curdle/Type.h"
#include "sstream"
#include "curdle/builtin.h"
#include "curdle/types/TypeType.h"
#include "curdle/values/ComptimeType.h"
#include "curdle/types/AnyType.h"
#include "curdle/curdle.h"
#include "curdle/names.h"

namespace cheese::curdle {

    bacteria::TypePtr Structure::get_bacteria_type(bacteria::nodes::BacteriaProgram *program) {
        cached_type = program->get_type(bacteria::BacteriaType::Type::Object, {}, {}, {}, {}, mangle(name));
        for (auto &child: fields) {
            cached_type->child_types.push_back(child.type->get_cached_type(program));
        }
        return cached_type;
    }

    void Structure::mark_type_references() {
        containedContext->mark();
        for (auto &field: fields) {
            field.type->mark();
        }
        for (auto &var: top_level_variables) {
            var.second.type->mark();
        }
        for (auto &interface: interfaces) {
            interface->mark();
        }
        for (auto &set: function_sets) {
            set.second->mark();
        }
        for (auto &var: comptime_variables) {
            var.second.type->mark();
            var.second.value->mark();
        }
    }


    void Structure::search_entry() {
        if (containedContext->globalContext->entry_function) return;
        for (auto &child: lazies) {
            if (child == nullptr) continue;
            if (auto as_import = dynamic_cast<parser::nodes::Import *>(child->node.get()); as_import) {
                resolve_lazy(child);
            }
            if (containedContext->globalContext->entry_function) return;
        }
        // Alright, now start heading down the tree :), this might start importing mixins, so we might need to find a better way to do this

        for (auto &var: comptime_variables) {
            auto type = var.second.value->type;
            if (auto as_type = dynamic_cast<TypeType *>(type); as_type) {
                auto value = dynamic_cast<ComptimeType *>(var.second.value);
                if (auto struct_value = dynamic_cast<Structure *>(value->typeValue); struct_value) {
                    struct_value->search_entry();
                }
            }
            if (containedContext->globalContext->entry_function) return;
        }
    }

    Structure::~Structure() {
        for (auto &lazy: lazies) {
            if (lazy == nullptr) continue;
            delete lazy;
            lazy = nullptr;
        }
    }

    template<typename T>
    requires std::is_base_of_v<Type, T>
    static gcref<ComptimeValue> create_from_type(cheese::project::GlobalContext *gctx, T *ref) {
        auto type = new ComptimeType{gctx, static_cast<Type *>(ref)};
        return {gctx->gc, type};
    }

    void Structure::resolve_lazy(LazyValue *&lazy) {
        auto ptr = lazy->node.get();
        auto &gc = containedContext->globalContext->gc;
        auto gctx = containedContext->globalContext;
#define WHEN_LAZY_IS(type, name) if (auto name = dynamic_cast<type*>(ptr); name)
        WHEN_LAZY_IS(parser::nodes::Import, pImport) {
            Structure *imported = containedContext->globalContext->import_structure(pImport->location, pImport->path,
                                                                                    containedContext->path.parent_path(),
                                                                                    containedContext->project_dir);
            auto n = lazy->name;
            lazy = nullptr;
            auto from_type = create_from_type(gctx, imported);
            ComptimeVariableInfo info{true, true, TypeType::get(gctx), from_type};
            comptime_variables[n] = info;
            return;
        }
        WHEN_LAZY_IS(parser::nodes::VariableDeclaration, pVariableDeclaration) {
            gcref<Type> result_type{gc, nullptr};
            Type *value_type = nullptr; // This is the cached type
            gcref<LocalContext> result_context{gc, nullptr};
            auto rctx = gc.gcnew<RuntimeContext>(containedContext, this);
            auto definition = (parser::nodes::VariableDefinition *) pVariableDeclaration->def.get();
            if (definition->type.has_value()) {
                try {
                    auto ty = containedContext->exec(definition->type.value(), rctx);
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
            bool ctime = result_type->get_comptimeness() != Comptimeness::Runtime || definition->flags.comptime;
            if (ctime) {
                try {
                    containedContext->push_structure_name(name.empty() ? lazy->name : name + "." + lazy->name);
                    auto value = containedContext->exec(pVariableDeclaration->value);
                    containedContext->pop_structure_name();
                    comptime_variables.insert({lazy->name, ComptimeVariableInfo{
                            definition->flags.pub != 0,
                            definition->flags.comptime != 0,
                            result_type,
                            value
                    }});
                    return;
                } catch (const NotComptimeError &e) {
                    gctx->raise(e.what(), pVariableDeclaration->location, error::ErrorCode::NotComptime);
                    return;
                } catch (const BadBuiltinCall &e) {
                    gctx->raise(e.what(), pVariableDeclaration->location, error::ErrorCode::BadBuiltinCall);
                    return;
                }
            } else {

            }

        }
        NOT_IMPL_FOR(typeid(*ptr).name());
#undef WHEN_LAZY_IS
    }

    Comptimeness Structure::get_comptimeness() {
        for (auto &field: fields) {
            if (field.type->get_comptimeness() != Comptimeness::Runtime) {
                return Comptimeness::Comptime;
            }
        }
        return Comptimeness::Runtime;
    }

    int32_t Structure::compare(Type *other, bool implicit) {
        if (other == this) return 0;
        if (auto s = dynamic_cast<Structure *>(other); s && fields.size() == s->fields.size()) {
            if (implicit_type || s->implicit_type) {
                auto result = 0;
                for (int i = 0; i < fields.size(); i++) {
                    if (fields[i].name != s->fields[i].name) return -1;
                    // As implicit stuff should be more loosely compared
                    auto comp = s->fields[i].type->compare(fields[i].type);
                    if (comp == -1) return -1;
                    result += comp;
                }
                return result;
            }
            if (is_tuple && s->is_tuple) {
                for (int i = 0; i < fields.size(); i++) {
                    if (fields[i].type->compare(s->fields[i].type) != 0) return -1;
                }
                return 0;
            }
        }
        return -1;
    }

    std::string Structure::to_string() {
        if (is_tuple) {
            std::stringstream ss{};
            ss << "struct(";
            for (int i = 0; i < fields.size(); i++) {
                ss << fields[i].type->to_string();
                if (i < fields.size() - 1) {
                    ss << " ";
                }
            }
            ss << ")";
            return ss.str();
        } else {
            return name;
        }
    }

    void Structure::resolve_by_name(const std::string &name) {
        for (auto &lazy: lazies) {
            if (lazy != nullptr && lazy->name == name) {
                resolve_lazy(lazy);
            }
        }
    }

    gcref<Type> Structure::peer(Type *other, cheese::project::GlobalContext *gctx) {
        if (other == this) return gcref{gctx->gc, this};
        if (dynamic_cast<AnyType *>(other)) return gcref{gctx->gc, this};
        auto &gc = gctx->gc;

        if (auto s = dynamic_cast<Structure *>(other); s && fields.size() == s->fields.size()) {
            if (implicit_type || s->implicit_type) {
                if (implicit_type && !s->implicit_type) return gcref{gc, this};
                if (s->implicit_type && !implicit_type) return gcref{gc, other};
                auto ty = gc.gcnew<Structure>(gctx->verify_name("::peer"), this->containedContext, gc);
                ty->is_tuple = is_tuple;
                ty->implicit_type = true;
                for (int i = 0; i < fields.size(); i++) {
                    if (fields[i].name != s->fields[i].name) return gcref<Type>{gc, nullptr};
                    // As implicit stuff should be more loosely compared
                    if (s->fields[i].type->compare(fields[i].type) == -1) return gcref<Type>{gc, nullptr};
                    ty->fields.push_back(StructureField{
                            "_" + std::to_string(i),
                            fields[i].type->peer(s->fields[i].type, gctx),
                            true
                    });
                }
                return ty;
            }
            if (is_tuple && s->is_tuple) {
                auto ty = gc.gcnew<Structure>(gctx->verify_name("::peer"), this->containedContext, gc);
                ty->is_tuple = true;
                ty->implicit_type = true;
                for (int i = 0; i < fields.size(); i++) {
                    if (fields[i].type->compare(s->fields[i].type) == -1) return gcref<Type>{gc, nullptr};
                    ty->fields.push_back(StructureField{
                            "_" + std::to_string(i),
                            fields[i].type->peer(s->fields[i].type, gctx),
                            true
                    });
                }
                return ty;
            }
        }
        return gcref<Type>{gc, nullptr};
    }

    Structure::Structure(std::string name, ComptimeContext *ctx, garbage_collector &gc) : name(name) {
        containedContext = gc.gcnew<ComptimeContext>(ctx, this);
    }
}
