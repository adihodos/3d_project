#pragma once

#include <D3D11.h>
#include "directx_shader.h"
#include "misc.h"
#include "win32_traits.h"

namespace game {
  class renderer;
}

namespace outer_limits {

class directx_vertex_shader : public directx_shader {
private :
  NO_CC_ASSIGN(directx_vertex_shader);

  base::scoped_ptr<ID3D11VertexShader, win32::traits::com_ptr>  shader_handle_;
  static const char* const kShaderProfileNames[];
public :
  enum vertex_profile {
    vertex_profile_4_0,
    vertex_profile_5_0
  };

  directx_vertex_shader(const char* name) 
    : directx_shader(name) {}

  bool compile_from_file(
    game::renderer* renderer,
    const char* file_name, 
    const char* entry_point, 
    int profile, 
    unsigned compile_flags
    );

  ID3D11VertexShader* get_shader_handle() const {
    return base::scoped_ptr_get(shader_handle_);
  }

  void bind_to_pipeline(game::renderer* renderer);
};

typedef directx_vertex_shader   vertex_shader_t;

} // namespace outer_limits