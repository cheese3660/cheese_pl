//
// Created by Lexi Allen on 3/30/2023.
//

#ifndef CHEESE_GARBAGECOLLECTION_H
#define CHEESE_GARBAGECOLLECTION_H

#include <vector>
#include <concepts>
#include <stdint.h>

namespace cheese::memory::garbage_collection {

    struct managed_object {
        bool marked = false;

        virtual void mark_references() = 0;

        void mark();

        virtual ~managed_object() = 0;
    };

    class garbage_collector;


    template<typename T> requires std::is_base_of_v<managed_object, T>
    struct gcref;

    // Simple linear GC for now :)
    class garbage_collector {
        std::vector<managed_object *> managed_objects{};
        std::vector<managed_object *> roots{};
        std::vector<managed_object *> in_scope_objects{};
        size_t frequency; // After how many allocations do we run garbage collection
        size_t allocations_since_last_sweep{0};
    public:

        explicit garbage_collector(size_t frequency) : frequency(frequency) {}

/// The returned value of this should never be stored in an object, you should unwrap it first
        template<class T, typename ...Args>
        requires std::is_base_of_v<managed_object, T> gcref<T> gcnew(Args &&... args);

        void add_in_scope_object(managed_object *object);

        void remove_in_scope_object(managed_object *object);

        void mark_and_sweep();

        void add_root_object(managed_object *object);

        void remove_root_object(managed_object *object);
    };

    // Maybe want to do a shared ptr thing, so that when this goes out of scope, y'know
    template<typename T> requires std::is_base_of_v<managed_object, T>
    struct gcref {
        garbage_collector &gc;
        size_t *ref_count;
        T *value;

        gcref(garbage_collector &gc, T *value) : gc(gc), value(value) {
            ref_count = new size_t{1};
            gc.add_in_scope_object(value);

        }

        gcref(const gcref<T> &other) : gc(other.gc) {
            ref_count = other.ref_count;
            *ref_count += 1;
            value = other.value;
        }

        gcref(gcref<T> &&other) : gc(other.gc) {
            ref_count = other.ref_count;
            value = other.value;
        }

        gcref<T> &operator=(gcref<T> &&other) = delete;

        gcref<T> &operator=(gcref<T> &other) = delete;

        T &operator*() {
            return *value;
        }

        T *operator->() {
            return value;
        }

        T *unwrap() {
            return value;
        }

    private:
        ~gcref() {
            *ref_count -= 1;
            if (*ref_count == 0) {
                gc.remove_in_scope_object(value);
            }
        }

    };

    template<class T, typename... Args>
    requires std::is_base_of_v<managed_object, T>
    gcref<T> garbage_collector::gcnew(Args &&... args) {
        auto obj = new T(std::forward<Args>(args)...);
        managed_objects.push_back(obj);
        auto ref = gcref{*this, obj};
        if (++allocations_since_last_sweep >= frequency) {
            mark_and_sweep();
        }
        return ref;
    }
};


#endif //CHEESE_GARBAGECOLLECTION_H
