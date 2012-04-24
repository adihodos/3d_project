#pragma once

#include "misc.h"
#include "scoped_ptr.h"
#include "win32_traits.h"

struct ID3D11Buffer;

namespace game {
  class renderer;
}

namespace outer_limits {

struct buffer_traits_t {
  enum buffer_usage_flags {
    bflag_gpu_read_write,
    bflag_gpu_read_cpu_noaccess,
    bflag_gpu_read_cpu_write,
    bflag_copy_from_gpu
  };

  enum buffer_cpu_access_flags {
    bcflag_cpu_read = 0x10000L,
    bcflag_cpu_write = 0x20000L
  };

  static int map_usage_flag_to_api_flag(buffer_usage_flags flag);

  static long map_usage_flag_to_cpu_access_flag(buffer_usage_flags flag);
};

namespace directx_details {

class directx_buffer_impl {
protected :
  NO_CC_ASSIGN(directx_buffer_impl);
  base::scoped_ptr<ID3D11Buffer, win32::traits::com_ptr>  handle_;

  enum buffer_type {
    buff_type_vertex,
    buff_type_index
  };

  directx_buffer_impl() {}

  ~directx_buffer_impl();

  bool initialize(
    game::renderer* renderer,
    buffer_type type,
    size_t data_size,
    size_t element_size,
    const void* initial_data, 
    buffer_traits_t::buffer_usage_flags use_flag
    );

  void update_data(
    game::renderer* renderer,
    const void* data,
    size_t size
    );

  ID3D11Buffer* get_buffer_handle() const {
    return base::scoped_ptr_get(handle_);
  }
};

} // namespace directx_details

} // namespace outer_limits