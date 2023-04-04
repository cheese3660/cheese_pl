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
        mark();
        sweep();
    }

    void garbage_collector::sweep() {
        size_t num_deleted = 0;
        for (auto &object: managed_objects) {
            if (object == nullptr) {
                num_deleted++;
                continue;
            }
            if (!object->marked) {
                num_deleted++;
                delete object;
                object = nullptr;
            }
        }
        // If >= 50% of objects are deleted, compact the vector for faster iteration
        if ((num_deleted << 1) >= managed_objects.size() && managed_objects.size() >= 16) {
            auto new_vec = std::vector<managed_object *>{};
            new_vec.reserve((managed_objects.size()) - num_deleted);
            for (auto object: managed_objects) {
                if (object != nullptr) new_vec.push_back(object);
            }
            managed_objects = std::move(new_vec);
        }
    }

    void garbage_collector::mark() {
        for (auto object: managed_objects) {
            if (object != nullptr)
                object->marked = false;
        }
        for (auto object: roots) {
            object->mark();
        }
        for (auto object: in_scope_objects) {
            object->mark();
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

    garbage_collector::~garbage_collector() {
        for (auto &ptr: managed_objects) {
            delete ptr;
        }
    }




    // This just gets a scoped reference to an object that exists in the garbage collector, hella useful when working with functions

    template<class T>
    requires std::is_base_of_v<managed_object, T>
    gcref<T> garbage_collector::get_scoped_ref(T *ptr) {
        return gcref<T>(*this, ptr);
    }

    void managed_object::mark() {
        if (marked) return; // Prevent cycles from occuring
        marked = true;
        mark_references();
    }
}