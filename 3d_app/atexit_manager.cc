#include "pch_hdr.h"
#include "atexit_manager.h"
#include "utility.h"

base::atexit_manager* base::at_exit_manager_global_instance;

base::atexit_manager::atexit_manager() {
    if (!at_exit_manager_global_instance)
        at_exit_manager_global_instance = this;
    else
        NOT_REACHED();
}

base::atexit_manager::~atexit_manager() {
    execute_all_callbacks();
}

void base::atexit_manager::register_callback(
    const base::callback_and_param_t& cbp
    )
{
    if (std::find(std::begin(registered_callbacks_), 
                  std::end(registered_callbacks_),
                  cbp) != std::end(registered_callbacks_)) {
        return;
    }

    registered_callbacks_.push_back(cbp);
}

void base::atexit_manager::execute_all_callbacks() {
    std::for_each(
        registered_callbacks_.rbegin(),
        registered_callbacks_.rend(),
        [](const callback_and_param_t& current_cb) {
            current_cb.callback_(current_cb.param_);
    });

    registered_callbacks_.clear();
}