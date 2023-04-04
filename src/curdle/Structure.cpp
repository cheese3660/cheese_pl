//
// Created by Lexi Allen on 3/30/2023.
//

#include "curdle/Structure.h"
#include "curdle/GlobalContext.h"
#include "curdle/Type.h"

namespace cheese::curdle {

    bacteria::TypePtr Structure::get_bacteria_type() {
        return cheese::bacteria::TypePtr{};
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
}
