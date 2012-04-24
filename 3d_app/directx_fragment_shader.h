#pragma once

#include <D3D11.h>
#include "directx_shader.h"
#include "misc.h"
#include "win32_traits.h"

namespace game {
  class renderer;
}

namespace outer_limits {

class directx_fragment_shader : public directx_shader {
private :
  NO_CC_ASSIGN(directx_fragment_shader);

  base::scoped_ptr<ID3D11PixelShader, win32::traits::com_ptr> shader_handle_;
  static const char* const kFragmentProfileNames[];

public :
  enum fragment_profile {
    fragment_profile_4_0,
    fragment_profile_5_0
  };

  directx_fragment_shader(const char* name)
    : directx_shader(name) {}

  ID3D11PixelShader* get_shader_handle() const {
    return base::scoped_ptr_get(shader_handle_);
  }

  bool compile_from_file(
    game::renderer* r,
    const char* file_name, 
    const char* entry_point, 
    int profile, 
    unsigned debug_flags
    );

  void bind_to_pipeline(game::renderer* renderer);
};

typedef directx_fragment_shader   fragment_shader_t;

} // namespace outer_limits