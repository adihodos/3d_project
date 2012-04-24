#include "pch_hdr.h"
#include "d3d_renderer.h"
#include "directx_misc.h"
#include "directx_shader.h"
#include "uniform_buffer_directx.h"

void outer_limits::directx_shader::set_uniform_buffer_data(
  outer_limits::uniform_buffer_dxbase* ubuff_dx
  )
{
  ubuff_data_t* buff_data = nullptr;
  for (size_t i = 0; i < shader_uniforms_.size(); ++i) {
    if (shader_uniforms_[i] == ubuff_dx->get_buffer_handle()) {
      buff_data = &shader_uniforms_[0] + i;
      break;
    }
  }

  if (buff_data) {
    buff_data->u_handle = ubuff_dx->get_buffer_handle();
    //
    // Doesn't make sense for a binding slot to change
    //buff_data->u_slot = ubuff_dx->get_binding_slot();
    return;
  }

  D3D11_SHADER_INPUT_BIND_DESC binding_info;
  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    shader_reflector_->GetResourceBindingDescByName(
      ubuff_dx->get_name(), &binding_info));
  
  if (FAILED(ret_code)) {
    NOT_REACHED_MSGA("Constant buffer with name %s does not exist!", 
                     ubuff_dx->get_name());
    return;
  }

  ubuff_dx->set_binding_slot(binding_info.BindPoint);
  shader_uniforms_.push_back(ubuff_data_t(ubuff_dx->get_binding_slot(), 
                                          ubuff_dx->get_buffer_handle()));

  std::sort(std::begin(shader_uniforms_), std::end(shader_uniforms_), 
            [](const ubuff_data_t& left, const ubuff_data_t& right) -> bool {
              return left.u_slot < right.u_slot;
  });
}

bool outer_limits::directx_shader::create_shader_reflection() {
  base::scoped_ptr_reset(shader_reflector_);

  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    ::D3DReflect(
      shader_bytecode_->GetBufferPointer(), 
      shader_bytecode_->GetBufferSize(),
      IID_ID3D11ShaderReflection,
      reinterpret_cast<void**>(base::scoped_ptr_get_ptr_ptr(shader_reflector_))
    ));

  return !!shader_reflector_;
}

bool outer_limits::directx_shader::compile_shader_to_bytecode(
  const char* file_name,
  const char* entry_point,
  const char* profile,
  unsigned compile_flags
  )
{
  base::scoped_ptr_reset(shader_bytecode_);

  using namespace directx_details;
  compile_result_t compile_result(compile_shader_from_file(
    file_name, entry_point, profile, compile_flags));
  if (!std::get<0>(compile_result))
    return false;

  base::scoped_ptr_reset(shader_bytecode_, std::get<1>(compile_result));
  return true;
}
