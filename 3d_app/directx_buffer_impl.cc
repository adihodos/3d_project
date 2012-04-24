#include "pch_hdr.h"
#include "d3d_renderer.h"
#include "directx_buffer_impl.h"
#include "directx_misc.h"
#include "utility.h"

int outer_limits::buffer_traits_t::map_usage_flag_to_api_flag(
  buffer_usage_flags flag
  ) {
  switch (flag) {
  case bflag_gpu_read_write :
    return D3D11_USAGE_DEFAULT;
    break;

  case bflag_gpu_read_cpu_noaccess :
    return D3D11_USAGE_IMMUTABLE;
    break;

  case bflag_gpu_read_cpu_write :
    return D3D11_USAGE_DYNAMIC;
    break;

  case bflag_copy_from_gpu :
    return D3D11_USAGE_STAGING;
    break;

  default :
    NOT_REACHED();
    return D3D11_USAGE_DEFAULT;
  }
}

long outer_limits::buffer_traits_t::map_usage_flag_to_cpu_access_flag(
  buffer_usage_flags flag
  )
{
  switch (flag) {
  case bflag_gpu_read_cpu_write :
    return D3D11_CPU_ACCESS_WRITE;
    break;

  case bflag_copy_from_gpu :
    return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    break;

  default :
    return 0;
  }
}

outer_limits::directx_details::directx_buffer_impl::~directx_buffer_impl() {}

bool outer_limits::directx_details::directx_buffer_impl::initialize(
  game::renderer* renderer,
  directx_buffer_impl::buffer_type type,
  size_t data_size,
  size_t element_size,
  const void* initial_data, 
  buffer_traits_t::buffer_usage_flags use_flag
  )
{
  const D3D11_BIND_FLAG kBindingFlag = 
    (type == buff_type_vertex ? D3D11_BIND_VERTEX_BUFFER 
                              : D3D11_BIND_INDEX_BUFFER);

  D3D11_BUFFER_DESC creation_data = {
    data_size,
    static_cast<D3D11_USAGE>(buffer_traits_t::map_usage_flag_to_api_flag(use_flag)),
    kBindingFlag,
    buffer_traits_t::map_usage_flag_to_cpu_access_flag(use_flag),
    0,
    element_size
  };

  D3D11_SUBRESOURCE_DATA init_data = { initial_data, 0, 0 };

  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    renderer->get_device()->CreateBuffer(
      &creation_data, initial_data ? &init_data : nullptr, 
      base::scoped_ptr_get_ptr_ptr(handle_)));

  return ret_code == S_OK;
}

void outer_limits::directx_details::directx_buffer_impl::update_data(
  game::renderer* renderer,
  const void* data,
  size_t size
  )
{
  resource_mapping res_mapping(
    renderer->get_device_context(), base::scoped_ptr_get(handle_),
    0, D3D11_MAP_WRITE_DISCARD
    );

  if (!res_mapping) {
    NOT_REACHED();
    return;
  }

  memcpy(res_mapping.get_mapping().pData, data, size);
}