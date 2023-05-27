//
// Created by Lexi Allen on 3/30/2023.
//

#include "curdle/Structure.h"
#include "curdle/GlobalContext.h"
#include "curdle/Type.h"
#include "sstream"
#include "curdle/builtin.h"

namespace cheese::curdle {

    bacteria::TypePtr Structure::get_bacteria_type() {
        cached_type = std::make_shared<bacteria::BacteriaType>(bacteria::BacteriaType::Type::Object);
        std::vector<bacteria::TypePtr> child_types;
        std::vector<bacteria::TypePtr> to_check_references;
        for (auto &child: fields) {
            child_types.push_back(child.type->get_cached_type());
        }
        // Now we do a cyclic reference finding algorithm to find if we have any non-weak references to this type in the list and we replace them



        *cached_type = bacteria::BacteriaType(bacteria::BacteriaType::Type::Object, 0,
                                              std::shared_ptr<bacteria::BacteriaType>{},
                                              std::vector<std::size_t>{},
                                              std::move(child_types));
        to_check_references.push_back(cached_type);
        // Treat this like a stack
        while (!to_check_references.empty()) {
            auto next_check = to_check_references.back();
            to_check_references.pop_back();
            if (next_check->type == bacteria::BacteriaType::Type::Reference) {
                if (next_check->subtype == cached_type) {
                    next_check->type = bacteria::BacteriaType::Type::WeakReference;
                    next_check->weak_reference = std::weak_ptr<bacteria::BacteriaType>(cached_type);
                    next_check->subtype = {}; // Clear it out to make sure it gets destroyed
                } else {
                    to_check_references.push_back(next_check->subtype);
                }
            }
            if (next_check->type == bacteria::BacteriaType::Type::Pointer) {
                if (next_check->subtype == cached_type) {
                    next_check->type = bacteria::BacteriaType::Type::WeakPointer;
                    next_check->weak_reference = std::weak_ptr<bacteria::BacteriaType>(cached_type);
                    next_check->subtype = {}; // Clear it out to make sure it gets destroyed
                } else {
                    to_check_references.push_back(next_check->subtype);
                }
            }
            if (next_check->type == bacteria::BacteriaType::Type::Slice) {
                if (next_check->subtype == cached_type) {
                    next_check->type = bacteria::BacteriaType::Type::WeakSlice;
                    next_check->weak_reference = std::weak_ptr<bacteria::BacteriaType>(cached_type);
                    next_check->subtype = {}; // Clear it out to make sure it gets destroyed
                } else {
                    to_check_references.push_back(next_check->subtype);
                }
            }

            if (next_check->type == bacteria::BacteriaType::Type::Array) {
                to_check_references.push_back(next_check->subtype);
            }
            if (next_check->type == bacteria::BacteriaType::Type::Object) {
                for (const auto &to_check: next_check->child_types) {
                    to_check_references.push_back(to_check);
                }
            }
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
    static gcref<ComptimeValue> create_from_type(garbage_collector &gc, T *ref) {
        auto type = new ComptimeType{gc, static_cast<Type *>(ref)};
        return gc.manage<ComptimeValue>(type);
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
            lazy = nullptr;
            auto from_type = create_from_type(gc, imported);
            ComptimeVariableInfo info{true, true, TypeType::get(gc), from_type};
            comptime_variables[lazy->name] = info;
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
        if (auto s = dynamic_cast<Structure *>(other); s && is_tuple && s->is_tuple &&
                                                       fields.size() == s->fields.size()) {
            for (int i = 0; i < fields.size(); i++) {
                if (fields[i].type->compare(s->fields[i].type) != 0) return -1;
            }
            return 0;
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

    Type *Structure::peer(Type *other, garbage_collector &gc) {
        if (compare(other) == 0) return this;
        if (dynamic_cast<AnyType *>(other)) return this;
        return nullptr;
    }

    Structure::Structure(std::string name, ComptimeContext *ctx, garbage_collector &gc) : name(name) {
        containedContext = gc.gcnew<ComptimeContext>(ctx, this);
    }
}
