//
// Created by Lexi Allen on 3/30/2023.
//

#ifndef CHEESE_GARBAGECOLLECTION_H
#define CHEESE_GARBAGECOLLECTION_H

#include <vector>
#include <concepts>
#include <iostream>

namespace cheese::memory::garbage_collection {

    struct managed_object {
        bool marked = false;

        virtual void mark_references() = 0;

        void mark();

        virtual ~managed_object() = default;
    };

    class garbage_collector;


    template<typename T>
    struct gcref;

//    template<typename T> requires std::is_base_of_v<managed_object, T>
//    struct gcvec : managed_object, std::vector<T *> {
//
//        ~gcvec() override = default;
//
//        void mark_references() override {
//            for (auto it = this->begin(); it != this->end(); ++it) {
//                *it->mark();
//            }
//        }
//    };

    // Simple linear GC for now :)
    class garbage_collector {
        std::vector<managed_object *> managed_objects{};
        std::vector<managed_object *> roots{};
        std::vector<managed_object *> in_scope_objects{};
        size_t frequency; // After how many allocations do we run garbage collection
        size_t allocations_since_last_sweep{0};

        void mark_and_sweep();

        void mark();

        void sweep();

    public:

        explicit garbage_collector(size_t frequency) : frequency(frequency) {}

/// The returned value of this should never be stored in an object, you should unwrap it first
        template<class T, typename ...Args>
        requires std::is_base_of_v<managed_object, T> gcref<T> gcnew(Args &&... args);

        template<class T>
        requires std::is_base_of_v<managed_object, T> gcref<T> get_scoped_ref(T *ptr);

        template<class T>
        requires std::is_base_of_v<managed_object, T> gcref<T> manage(T *ptr);

        void add_in_scope_object(managed_object *object);

        void remove_in_scope_object(managed_object *object);

        void add_root_object(managed_object *object);

        void remove_root_object(managed_object *object);

        ~garbage_collector();
    };

    // Make this a unique ptr, such that it doesn't get copied to a field and such accidentally.
    template<typename T>
    struct gcref {
        static_assert(std::is_base_of_v<managed_object, T>, "Class must be of type managed_object");
        garbage_collector &gc;
        T *value;

        gcref(garbage_collector &gc, T *value) : gc(gc), value(value) {
            if (value == nullptr) return;
            gc.add_in_scope_object(value);
        }


        gcref(const gcref<T> &other) = delete;

        gcref(gcref<T> &&other) noexcept: gc(other.gc) {
            value = other.value;
            other.value = nullptr;
        }

        template<typename V>
        gcref(gcref<V> &&other) noexcept: gc(other.gc) {
            static_assert(std::is_base_of_v<T, V>, "V must derive from T");
            value = other.value;
        }

        gcref<T> &operator=(gcref<T> &&other) noexcept {
            if (this == &other) return *this;
            gc.remove_in_scope_object(value);
            value = std::move(other.value);
            other.value = nullptr;
            return *this;
        }

        gcref<T> &operator=(gcref<T> &other) = delete;

        T &operator*() {
            return *value;
        }

        T *operator->() {
            return value;
        }

        T *get() const {
            return value;
        }

        explicit operator bool() const {
            return value;
        }

        ~gcref() {
            if (value == nullptr) return;
            gc.remove_in_scope_object(value);
        }

        operator T *() const { // NOLINT(google-explicit-constructor)
            return value;
        }

    };

    template<class T, typename... Args>
    requires std::is_base_of_v<managed_object, T>
    gcref<T> garbage_collector::gcnew(Args &&... args) {
        auto obj = new T(std::forward<Args>(args)...);
//        std::cout << "Creating a: " << typeid(*obj).name() << "\n";
        managed_objects.push_back(obj);
        auto ref = gcref{*this, obj};
        if (++allocations_since_last_sweep >= frequency) {
            mark_and_sweep();
            allocations_since_last_sweep = 0;
        }
        return ref;
    }

    template<class T>
    requires std::is_base_of_v<managed_object, T>
    gcref<T> garbage_collector::manage(T *ptr) {
        managed_objects.push_back(reinterpret_cast<managed_object *>(ptr));
        return gcref<T>(*this, ptr);
    }

};


#endif //CHEESE_GARBAGECOLLECTION_H
