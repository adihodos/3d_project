#include "pch_hdr.h"
#include "d3d_renderer.h"
#include "utility.h"
#include "uniform_buffer_directx.h"

bool outer_limits::uniform_buffer_dxbase::initialize(
  game::renderer* r,
  size_t buff_size,
  size_t stride_size
  )
{
  assert(!buff_ptr_ && "buffer is already initialized");

  D3D11_BUFFER_DESC buffer_des = {
    buff_size,
    D3D11_USAGE_DEFAULT,
    D3D11_BIND_CONSTANT_BUFFER,
    D3D11_CPU_ACCESS_WRITE,
    0,
    stride_size
  };

  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    r->get_device()->CreateBuffer(
      &buffer_des, nullptr, base::scoped_ptr_get_ptr_ptr(buff_ptr_)
      ));

  return ret_code == S_OK;
}

void outer_limits::uniform_buffer_dxbase::sync_with_gpu(
  game::renderer* rend,
  const void* data
  ) {
    rend->get_device_context()->UpdateSubresource(
      base::scoped_ptr_get(buff_ptr_), 0, nullptr,
      data, 0, 0);
}