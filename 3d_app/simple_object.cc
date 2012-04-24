#include "pch_hdr.h"
#include <gfx/color.h>
#include <gfx/vector3.h>
#include <gfx/matrix3X3.h>
#include <gfx/matrix4X4.h>
#include "d3d_renderer.h"
#include "directx_misc.h"
#include "fragment_shader.h"
#include "ifs_model_loader.h"
#include "index_buffer.h"
#include "input_layout.h"
#include "primitive_topology_types.h"
#include "simple_object.h"
#include "scoped_ptr.h"
#include "uniform_buffer.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "win32_traits.h"

using namespace win32::traits;

class directx_shader_resource {
public :
  const char* get_name() const {
    return "unknown resource";
  }
};

class opengl_shader_resource {
  const char* get_name() const {
    return "unknown resource";
  }
};

#if defined(OPENGL_BUILDPASS)
typedef opengl_shader_resource shader_resource_t;
#else
typedef directx_shader_resource shader_resource_t;
#endif

namespace {

struct vertex_t {
  gfx::vector3F   vx_position;
  gfx::color      vx_color;

  vertex_t() {}

  vertex_t(const gfx::vector3F& pos, const gfx::color& color)
      : vx_position(pos), vx_color(color) {}
};

}

template<int dxtype>
struct mapper;

template<>
struct mapper<DXGI_FORMAT_R32G32B32A32_FLOAT> {
  typedef float element_t;
  static const size_t element_cout = 3;
};

/*
void glVertexAttribPointer(
GLuint  index,  GLint  size,  GLenum  type,  GLboolean  normalized,  
GLsizei  stride,  const GLvoid *  pointer
);
*/

class layout_descriptor_builder {
private :
  struct vertex_element_descriptor_t {
  const char* e_semantic_name;
  unsigned    e_semantic_index;
  unsigned    e_index;
  int         e_components;
  int         e_format;
  int         e_element_type;
  unsigned    e_offset;
  unsigned    e_input_slot;
};

public :
};

struct test::simple_object::implementation_details {
private :
    NO_CC_ASSIGN(implementation_details);

    struct uniforms_t {
      gfx::matrix_4X4F worldMatrix;
      gfx::matrix_4X4F worldViewProjectionMatrix;
    };

public :
    
    outer_limits::input_layout_t                    input_layout_;
    outer_limits::vertex_buffer_t                   vertex_buffer_;
    outer_limits::index_buffer_t                    index_buffer_;
    unsigned int                                    num_indices_;
    outer_limits::vertex_shader_t                   vertex_shader_;
    outer_limits::fragment_shader_t                 fragment_shader_;
    
    gfx::matrix_4X4F                                          model2world_;
    outer_limits::ubuff_rebind<uniforms_t>::uniform_buffer_t  uniforms_buffer_;

    implementation_details() 
        : num_indices_(0),
          vertex_shader_("simple_vertex_shader"),
          fragment_shader_("simple_fragment_shader"),
          model2world_(gfx::matrix_4X4F::identity),
          uniforms_buffer_("wvp_transforms", uniforms_t()) {}
};

test::simple_object::simple_object() : impl_(new implementation_details) {}

test::simple_object::~simple_object() {}

void test::simple_object::initialize(game::renderer* renderer) {
    const char* kVertexShaderFile = "C:\\temp\\shader_cache\\p1\\vertex_shader.hlsl";

    const UINT kShaderCompileFlags = 
      D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS |
      D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_DEBUG |
      D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    if (!impl_->vertex_shader_.compile_from_file(
      renderer,
      kVertexShaderFile, "vs_main", 
      outer_limits::vertex_shader_t::vertex_profile_5_0,
      kShaderCompileFlags
      )) {
      NOT_REACHED_MSG(L"Failed to compile vertex shader");
      return;
    }

    D3D11_INPUT_ELEMENT_DESC vertex_format_description[] = {
      { 
        "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
      },
      {
        "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
      }
    };

    if (!impl_->input_layout_.initialize(
      renderer,
      vertex_format_description, _countof(vertex_format_description),
      &impl_->vertex_shader_
      )) {
      NOT_REACHED_MSG(L"Failed to initialize input layout!");
      return;
    }

    const char* kModelDataFile = "C:\\temp\\model_cache\\ifs\\hind24h.ifs";
    base::model_format_handlers::IFSLoader model_loader;
    if (!model_loader.loadModel(kModelDataFile)) {
        NOT_REACHED();
        return;
    }

    impl_->num_indices_ = model_loader.getIndexCount();

    std::vector<vertex_t> model_vertices;
    model_vertices.reserve(model_loader.getVertexCount());
    std::transform(
        model_loader.getVertexListPointer(),
        model_loader.getVertexListPointer() + model_loader.getVertexCount(),
        std::back_inserter(model_vertices),
        [](const gfx::vector3F& v3) {
            return vertex_t(v3, gfx::color(0.0f, 0.36f, 0.73f));
    });

    if (!impl_->vertex_buffer_.initialize(
      game::renderer_instance_t::get(),
      model_vertices.size() * sizeof(model_vertices[0]),
      sizeof(model_vertices[0]),
      &model_vertices[0])) {
        NOT_REACHED();
        return;
    }

    if (!impl_->index_buffer_.initialize(
      game::renderer_instance_t::get(),
      model_loader.getIndexCount() * sizeof(DWORD),
      sizeof(DWORD),
      model_loader.getIndexListPointer()
      )) {
        NOT_REACHED();
        return;
    }
}

void test::simple_object::draw(
  game::renderer* r, 
  const gfx::matrix_4X4F& view_proj_transform
  ) {
  impl_->uniforms_buffer_.get_buffered_data().worldMatrix = impl_->model2world_;
  impl_->uniforms_buffer_.get_buffered_data().worldViewProjectionMatrix = 
    view_proj_transform * impl_->model2world_;
  impl_->uniforms_buffer_.sync_with_gpu(r);
    
  r->ia_stage_set_input_layout(&impl_->input_layout_);
  unsigned strides = sizeof(vertex_t);
  r->ia_stage_set_vertex_buffers(0, 1, &impl_->vertex_buffer_, &strides);
  r->ia_stage_set_index_buffer(&impl_->index_buffer_);
  r->ia_stage_set_primitive_topology(
    outer_limits::primitive_topology_triangle_list
    );

  impl_->vertex_shader_.set_uniform_buffer_data(&impl_->uniforms_buffer_);
  impl_->vertex_shader_.bind_to_pipeline(r);
  impl_->fragment_shader_.bind_to_pipeline(r);

  r->draw_indexed(impl_->num_indices_);
}