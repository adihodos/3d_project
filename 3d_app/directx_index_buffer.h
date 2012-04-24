#pragma once

#include "directx_buffer_impl.h"
#include "misc.h"

namespace outer_limits {

class directx_index_buffer : private directx_details::directx_buffer_impl {
private :
  NO_CC_ASSIGN(directx_index_buffer);

public :
  using directx_buffer_impl::get_buffer_handle;
  using directx_buffer_impl::update_data;

  directx_index_buffer() {}

  bool initialize(
    game::renderer* renderer,
    size_t data_size,
    size_t element_size,
    const void* initial_data = nullptr, 
    buffer_traits_t::buffer_usage_flags use_flag = buffer_traits_t::bflag_gpu_read_cpu_noaccess
    ) {
    return directx_buffer_impl::initialize(
      renderer, directx_buffer_impl::buff_type_index, data_size,
      element_size, initial_data, use_flag
      );
  }

  DXGI_FORMAT get_element_format() const {
    return DXGI_FORMAT_R32_UINT;
  }

  /*
  void update_data(
    game::renderer* renderer,
    const void* data,
    size_t size
    ) {
      directx_buffer_impl::update_data(renderer, data, size);
  }
  */
};

typedef directx_index_buffer index_buffer_t;

} // namespace outer_limits
