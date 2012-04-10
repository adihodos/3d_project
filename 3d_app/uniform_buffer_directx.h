#pragma once

#include <cassert>
#include <D3D11.h>
#include "utility.h"
#include "misc.h"
#include "renderer.h"
#include "scoped_ptr.h"
#include "win32_traits.h"

namespace renderer {

template<typename T>
class uniform_buffer_directx {
private :
  NO_CC_ASSIGN(uniform_buffer_directx);

  const char*                                               name_;
  T                                                         uniform_data_;
  base::scoped_ptr<ID3D11Buffer, win32::traits::com_ptr>    buff_ptr_;
  
public :
  uniform_buffer_directx(const char* name) : name_(name) {}

  uniform_buffer_directx(const char* name, const T& uniform_data) 
    : name_(name),
      uniform_data_(uniform_data) {}

  T& get_buffered_data() {
    return uniform_data_;
  }

  const T& get_buffered_data() const {
    return uniform_data_;
  }

  bool initialize(game::renderer*);

  void sync_with_gpu(game::renderer*);

  ID3D11Buffer* get_buffer_handle() const {
    return base::scoped_ptr_get(buff_ptr_);
  }

  const char* get_name() const {
    return name_;
  }
};

template<typename T>
bool uniform_buffer_directx<T>::initialize(game::renderer* r) {
  assert(!buff_ptr_ && "buffer is already initialized");

  D3D11_BUFFER_DESC buffer_des = {
    sizeof(T),
    D3D11_USAGE_DEFAULT,
    D3D11_BIND_CONSTANT_BUFFER,
    D3D11_CPU_ACCESS_WRITE,
    0,
    sizeof(T)
  };

  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    r->get_device()->CreateBuffer(
      &buffer_des, nullptr, base::scoped_ptr_get_ptr_ptr(buff_ptr_)
      ));

  return ret_code == S_OK;
}

template<typename T>
void uniform_buffer_directx<T>::sync_with_gpu(game::renderer* r) {
  r->get_device_context()->UpdateSubresource(
    base::scoped_ptr_get(buff_ptr_), 0, nullptr, &uniform_data_, 0, 0
    );
}

} // namespace renderer