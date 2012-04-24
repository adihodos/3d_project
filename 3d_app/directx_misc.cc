#include "pch_hdr.h"
#include "directx_misc.h"
#include "scoped_ptr.h"
#include "utility.h"
#include "win32_traits.h"

using namespace win32::traits;

outer_limits::directx_details::compile_result_t
outer_limits::directx_details::compile_shader_from_file(
  const char* file_name, 
  const char* function_name, 
  const char* profile,
  unsigned int compile_flags
  )
{
  base::scoped_ptr<ID3D10Blob, com_ptr> blob_bytecode;
  base::scoped_ptr<ID3D10Blob, com_ptr> blob_msg;
  HRESULT ret_code;
  CHECK_D3D(
      &ret_code,
      ::D3DX11CompileFromFileA(file_name, nullptr, nullptr, function_name,
                                profile, compile_flags, 0, nullptr,
                                scoped_ptr_get_ptr_ptr(blob_bytecode),
                                scoped_ptr_get_ptr_ptr(blob_msg),
                                nullptr));
  if (FAILED(ret_code)) {
      OUTPUT_DBG_MSGA(
          "Failed to compile shader, error = %s", 
          blob_msg->GetBufferPointer() ? 
              reinterpret_cast<const char*>(blob_msg->GetBufferPointer()) 
              : "unknown"
              );
      return compile_result_t(false, nullptr);
  }

  return compile_result_t(true, scoped_ptr_release(blob_bytecode));
}

outer_limits::directx_details::resource_mapping::resource_mapping(
  ID3D11DeviceContext* device_ctx, 
  ID3D11Resource* resource,
  unsigned subresource, 
  D3D11_MAP mapping_type
  )
  : device_context_(device_ctx),
    mapped_resource_(resource),
    subresource_(subresource)
{
  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    device_context_->Map(mapped_resource_, subresource_, 
                         mapping_type, 0, &mapping_));
  succeeded_ = (ret_code == S_OK);
}