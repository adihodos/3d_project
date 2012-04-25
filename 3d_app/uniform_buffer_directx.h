#pragma once

#include <D3D11.h>
#include "misc.h"
#include "scoped_ptr.h"
#include "win32_traits.h"

namespace game {
  class renderer;
}

namespace outer_limits {

class uniform_buffer_dxbase {
protected :
  unsigned int                                              bind_slot_;
  const char*                                               name_;
  base::scoped_ptr<ID3D11Buffer, win32::traits::com_ptr>    buff_ptr_;

  bool initialize(game::renderer* r, size_t buff_size, size_t stride_size);

  void sync_with_gpu(game::renderer*, const void*);

  uniform_buffer_dxbase(const char* name) : name_(name) {}
public :  
  ~uniform_buffer_dxbase() {}

  unsigned int get_binding_slot() const {
    return bind_slot_;
  }

  void set_binding_slot(unsigned int slot) {
    bind_slot_ = slot;
  }

  const char* get_name() const {
    return name_;
  }

  ID3D11Buffer* get_buffer_handle() const {
    return base::scoped_ptr_get(buff_ptr_);
  }
};

template<typename T>
class uniform_buffer_directx : public uniform_buffer_dxbase {
private :
  NO_CC_ASSIGN(uniform_buffer_directx);

  T                                                         uniform_data_;  
public :
  uniform_buffer_directx(const char* name) : uniform_buffer_dxbase(name) {
    static_assert(
      (sizeof(T) % 16 == 0) && 
      "For Direct3D, the size of the constant buffer must be a multiple of 16"
      );
  }

  uniform_buffer_directx(const char* name, const T& uniform_data) 
    : uniform_buffer_dxbase(name),
      uniform_data_(uniform_data) {
    static_assert(
      (sizeof(T) % 16 == 0), 
      "For Direct3D, the size of the constant buffer must be a multiple of 16"
      );
  }

  T& get_buffered_data() {
    return uniform_data_;
  }

  const T& get_buffered_data() const {
    return uniform_data_;
  }

  bool initialize(game::renderer* r) {
    return uniform_buffer_dxbase::initialize(r, sizeof(T), sizeof(T));
  }

  void sync_with_gpu(game::renderer* r) {
    return uniform_buffer_dxbase::sync_with_gpu(r, &uniform_data_);
  }
};

} // namespace outer_limits