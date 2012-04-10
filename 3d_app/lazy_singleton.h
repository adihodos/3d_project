#pragma once

#include "atexit_manager.h"
#include "misc.h"
#include "utility.h"

namespace base {

template<typename T>
class singleton_life_traits {
protected :
    ~singleton_life_traits() {}

public :
    static T* construct(void* mem_ptr) {
        return new(mem_ptr) T();
    }

    static void destruct(void* obj_ptr) {
        static_cast<T*>(obj_ptr)->~T();
    }
};

template<typename Ty>
class lazy_singleton {
private :
    union s_data_t {
        struct dummy_t {
        } align_helper;
        unsigned char storage_area[sizeof(Ty)];
    };
    static s_data_t s_data_;
    static bool initialized_;

    static void initialize();

protected :
    ~lazy_singleton() {}

public :
    static Ty* get() {
        if (!initialized_)
            initialize();

        return reinterpret_cast<Ty*>(s_data_.storage_area);
    }
};

template<typename Ty>
typename lazy_singleton<Ty>::s_data_t lazy_singleton<Ty>::s_data_;

template<typename Ty>
bool lazy_singleton<Ty>::initialized_ = false;

template<typename Ty>
void lazy_singleton<Ty>::initialize() {
    if (!initialized_) {
        Ty::construct(reinterpret_cast<void*>(s_data_.storage_area));
        at_exit_manager_global_instance->register_callback(
            callback_and_param_t(&Ty::destruct,
                                 reinterpret_cast<Ty*>(s_data_.storage_area)));
        initialized_ = true;
        return;
    }

    NOT_REACHED();
}

} // namespace base