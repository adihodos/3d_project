#pragma once

#include <D3D11.h>

//class ID3D10Blob;

#include <tuple>

namespace outer_limits {

namespace directx_details {

typedef std::tuple<bool, ID3D10Blob*> compile_result_t;

compile_result_t
compile_shader_from_file(
    const char* file_name, 
    const char* function_name, 
    const char* profile,
    unsigned int compile_flags
    );

class resource_mapping {
private :
  ID3D11DeviceContext*      device_context_;
  ID3D11Resource*           mapped_resource_;
  unsigned                  subresource_;
  D3D11_MAPPED_SUBRESOURCE  mapping_;
  bool                      succeeded_;

public :
  resource_mapping(
    ID3D11DeviceContext* device_ctx, 
    ID3D11Resource* resource,
    unsigned subresource, 
    D3D11_MAP mapping_type
    );

  ~resource_mapping() {
    if (succeeded_) {
      device_context_->Unmap(mapped_resource_, subresource_);
    }
  }

  bool operator!() const {
    return !succeeded_;
  }

  D3D11_MAPPED_SUBRESOURCE& get_mapping() {
    assert(succeeded_);
    return mapping_;
  }

  const D3D11_MAPPED_SUBRESOURCE& get_mapping() const {
    assert(succeeded_);
    return mapping_;
  }
};

} // namespace directx_details

} // namespace outer_limits