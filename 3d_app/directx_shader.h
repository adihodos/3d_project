#pragma once

#include <D3D11.h>
#include <vector>
#include "scoped_ptr.h"
#include "misc.h"
#include "win32_traits.h"

namespace game {
class renderer;
}

namespace outer_limits {

class uniform_buffer_dxbase;

class directx_shader {
private :
  NO_CC_ASSIGN(directx_shader);

protected :
  struct ubuff_data_t {
    unsigned int  u_slot;
    ID3D11Buffer* u_handle;

    ubuff_data_t(unsigned int slot, ID3D11Buffer* buff_handle)
      : u_slot(slot), u_handle(buff_handle) {}

    bool operator==(unsigned int slot) const {
      return u_slot == slot;
    }

    bool operator==(const ID3D11Buffer* buff) const {
      return u_handle == buff;
    }
  };  

  base::scoped_ptr<ID3D11ShaderReflection, win32::traits::com_ptr>    shader_reflector_;
  base::scoped_ptr<ID3D10Blob, win32::traits::com_ptr>                shader_bytecode_;
  std::vector<ubuff_data_t>                                           shader_uniforms_;
  const char*                                                         shader_name_;

  bool create_shader_reflection();

  bool compile_shader_to_bytecode(
    const char* shader_file,
    const char* entry_point,
    const char* shader_profile,
    unsigned int compile_flags
    );

  directx_shader(const char* name) : shader_name_(name) {}

public :
  virtual ~directx_shader() {}

  virtual bool compile_from_file(
    game::renderer* r,
    const char* file_name, 
    const char* entry_point, 
    int profile, 
    unsigned debug_flags
    ) = 0;

  virtual void bind_to_pipeline(game::renderer*) = 0;

  //virtual void set_shader_resource(shader_resource_t*) = 0;

  //virtual shader_resource_t* get_shader_resource(const char*) = 0;

  const char* get_name() const {
    shader_name_;
  }

  ID3D11ShaderReflection* get_reflector() const {
    return base::scoped_ptr_get(shader_reflector_);
  }

  ID3D10Blob* get_bytecode() const {
    return base::scoped_ptr_get(shader_bytecode_);
  }

  void set_uniform_buffer_data(outer_limits::uniform_buffer_dxbase* ubuff_dx);
};

} // namespace outer_limits