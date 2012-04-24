#pragma once

#include <D3D11.h>
#include "misc.h"
#include "scoped_ptr.h"
#include "win32_traits.h"

namespace game {
  class renderer;
}

namespace outer_limits {

class directx_vertex_shader;

class directx_input_layout {
private :
  NO_CC_ASSIGN(directx_input_layout);
  base::scoped_ptr<ID3D11InputLayout, win32::traits::com_ptr> layout_handle_;

public :
  directx_input_layout() {}

  bool initialize(
    game::renderer* renderer,
    const D3D11_INPUT_ELEMENT_DESC* element, 
    size_t count, 
    outer_limits::directx_vertex_shader* vxshader
    );

  bool initialize(
    game::renderer* renderer,
    const char* description_file,
    outer_limits::directx_vertex_shader* vxshader
    );

  ID3D11InputLayout* get_layout_handle() const {
    return base::scoped_ptr_get(layout_handle_);
  }
};

typedef directx_input_layout  input_layout_t;

} // namespace outer_limits