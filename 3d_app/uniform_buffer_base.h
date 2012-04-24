#pragma once

namespace renderer {

class uniform_buffer_base {
protected :
  unsigned int  bind_slot_;
  const char*   name_;

  ~uniform_buffer_base() {}

public :
  uniform_buffer_base(const char* name) : name_(name) {}
  
  unsigned int get_binding_slot() const {
    return bind_slot_;
  }

  void set_binding_slot(unsigned int slot) {
    bind_slot_ = slot;
  }

  const char* get_name() const {
    return name_;
  }
};

} // namespace renderer