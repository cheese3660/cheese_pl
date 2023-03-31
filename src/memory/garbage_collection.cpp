//
// Created by Lexi Allen on 3/30/2023.
//
#include "memory/garbage_collection.h"

namespace cheese::memory::garbage_collection {
    void garbage_collector::add_in_scope_object(managed_object *object) {
        in_scope_objects.push_back(object);
    }

    void garbage_collector::remove_in_scope_object(managed_object *object) {
        for (int i = 0; i < in_scope_objects.size(); i++) {
            if (in_scope_objects[i] == object) {
                in_scope_objects.erase(in_scope_objects.begin() + i);
                break;
            }
        }
    }

    void garbage_collector::mark_and_sweep() {
        for (auto &object: managed_objects) {
            object->marked = false;
        }
        for (auto &object: roots) {
            object->mark();
        }
        for (auto &object: in_scope_objects) {
            object->mark();
        }
        for (ptrdiff_t i = static_cast<ptrdiff_t>(managed_objects.size()) - 1; i >= 0; i++) {
            if (!managed_objects[i]->marked) {
                delete managed_objects[i];
                managed_objects.erase(managed_objects.begin() + i);
            }
        }
    }

    void garbage_collector::add_root_object(managed_object *object) {
        roots.push_back(object);
    }

    void garbage_collector::remove_root_object(managed_object *object) {
        for (int i = 0; i < roots.size(); i++) {
            if (roots[i] == object) {
                roots.erase(roots.begin() + i);
                break;
            }
        }
    }

    void managed_object::mark() {
        if (marked) return; // Prevent cycles from occuring
        marked = true;
        mark_references();
    }
}