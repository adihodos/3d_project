#pragma once

#include <vector>
#include "misc.h"

namespace base {

struct callback_and_param_t {
    typedef void (*callback_t)(void*);

    callback_t  callback_;
    void*       param_;

    callback_and_param_t(callback_t callback, void* param)
        : callback_(callback), param_(param) {}
};

inline
bool 
operator==(
    const callback_and_param_t& left, 
    const callback_and_param_t& right
    )
{
    return left.callback_ == right.callback_ && left.param_ == right.param_;
}

inline
bool
operator!=(
    const callback_and_param_t& left, 
    const callback_and_param_t& right
    )
{
    return !(left == right);
}

class atexit_manager {
private :
    NO_CC_ASSIGN(atexit_manager);

    std::vector<callback_and_param_t> registered_callbacks_;

public :
    atexit_manager();

    ~atexit_manager();

    void register_callback(const callback_and_param_t& cbp);

    void execute_all_callbacks();
};

extern atexit_manager* at_exit_manager_global_instance;

} // namespace base