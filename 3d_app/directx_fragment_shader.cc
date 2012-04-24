#include "pch_hdr.h"
#include "d3d_renderer.h"
#include "directx_fragment_shader.h"
#include "utility.h"
#include "win32_traits.h"

using namespace win32::traits;

const char* const 
outer_limits::directx_fragment_shader::kFragmentProfileNames[] = {
  "ps_4_0",
  "ps_5_0"
};

bool outer_limits::directx_fragment_shader::compile_from_file(
  game::renderer* r,
  const char* file_name, 
  const char* entry_point, 
  int profile, 
  unsigned compile_flags
  )
{
  base::scoped_ptr_reset(shader_handle_);
  base::scoped_ptr_reset(shader_reflector_);
  base::scoped_ptr_reset(shader_bytecode_);

  if (!directx_shader::compile_shader_to_bytecode(
        file_name, entry_point, 
        directx_fragment_shader::kFragmentProfileNames[profile],
        compile_flags) ||
      !directx_shader::create_shader_reflection()) {
    return false;
  }

  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    r->get_device()->CreatePixelShader(
      shader_bytecode_->GetBufferPointer(),
      shader_bytecode_->GetBufferSize(),
      nullptr,
      base::scoped_ptr_get_ptr_ptr(shader_handle_)
    ));

  return ret_code == S_OK;
}

void outer_limits::directx_fragment_shader::bind_to_pipeline(
  game::renderer* renderer
  ) {
  renderer->get_device_context()->PSSetShader(
    base::scoped_ptr_get(shader_handle_), nullptr, 0);

  std::vector<ID3D11Buffer*> constant_buffer_list(shader_uniforms_.size());
  std::transform(std::begin(shader_uniforms_), std::end(shader_uniforms_),
                 std::begin(constant_buffer_list),
                 [](const directx_shader::ubuff_data_t& cbuff_data) {
                   return cbuff_data.u_handle;
  });

  renderer->get_device_context()->PSSetConstantBuffers(
    shader_uniforms_[0].u_slot, constant_buffer_list.size(), 
    &constant_buffer_list[0]);

  //
  // samplers
  //
  // resource views
}