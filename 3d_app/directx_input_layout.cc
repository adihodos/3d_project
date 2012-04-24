#include "pch_hdr.h"
#include "d3d_renderer.h"
#include "directx_vertex_shader.h"
#include "utility.h"
#include "directx_input_layout.h"

bool outer_limits::directx_input_layout::initialize(
  game::renderer* renderer,
  const D3D11_INPUT_ELEMENT_DESC* element, 
  size_t count, 
  outer_limits::directx_vertex_shader* vxshader
  )
{
  assert(!layout_handle_ && "Layout object already initialized!");
  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    renderer->get_device()->CreateInputLayout(
      element, count, 
      vxshader->get_bytecode()->GetBufferPointer(),
      vxshader->get_bytecode()->GetBufferSize(),
      base::scoped_ptr_get_ptr_ptr(layout_handle_)
    ));

  return ret_code == S_OK;
}

namespace {
  struct file_deleter_t {
    void operator()(FILE* fp) const {
      if (fp)
        ::fclose(fp);
    }
  };

  struct dupe_str_deleter_t {
    void operator()(char* str) const {
      ::free(str);
    }
  };
}

bool outer_limits::directx_input_layout::initialize(
  game::renderer* renderer,
  const char* description_file,
  outer_limits::directx_vertex_shader* vxshader
  )
{
  std::string desc_file(description_file);
  desc_file.append(".dxdesc");

  std::unique_ptr<FILE, file_deleter_t> file_handle(::fopen(desc_file.c_str(), "r"));
  if (!file_handle) {
    OUTPUT_DBG_MSGA("Failed to open file %s", desc_file.c_str());
    return false;
  }

  std::vector<std::unique_ptr<char*, dupe_str_deleter_t>> semantics;

  enum state {
    state_ok,
    state_tag,
    state_semantic,
    state_semantic_index,
    state_format,
    state_slot,
    state_offset,
    state_slot_class,
    state_data_step_rate,
    state_error
  };

  int current_state = state_ok;
  for (; current_state != state_error;) {
    char buff_line[1024];
    if (!::fgets(buff_line, _countof(buff_line), file_handle.get()))
      break;

    if (!::strcmp("[element]", buff_line)) {
    }
  }
}