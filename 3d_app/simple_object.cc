#include "pch_hdr.h"
#include <gfx/color.h>
#include <gfx/vector3.h>
#include <gfx/matrix3X3.h>
#include <gfx/matrix4X4.h>
#include "d3d_renderer.h"
#include "ifs_model_loader.h"
#include "simple_object.h"
#include "scoped_ptr.h"
#include "uniform_buffer.h"
#include "win32_traits.h"

using namespace win32::traits;

namespace d3d_details {

typedef std::tuple<bool, ID3D10Blob*> compile_result_t;

compile_result_t
compile_shader_from_file(
    const char* file_name, 
    const char* function_name, 
    const char* profile,
    UINT compile_flags
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

} // namespace d3d_details

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

class IGPUShader {
public :
  virtual ~IGPUShader() {}

  virtual bool compile_from_file(
    const char* file_name, 
    const char* entry_point, 
    int profile, 
    unsigned debug_flags
    ) = 0;

  virtual void bind_to_pipeline(game::renderer*) = 0;

  virtual void set_shader_resource(shader_resource_t*) = 0;

  virtual shader_resource_t* get_shader_resource(const char*) = 0;
};

class DirectXShader : public IGPUShader {
protected :
  base::scoped_ptr<ID3D11ShaderReflection, com_ptr> shader_reflector_;
  base::scoped_ptr<ID3D10Blob, com_ptr>             shader_bytecode_;

  bool create_shader_reflection();

  bool compile_shader_to_bytecode(
    const char* shader_file,
    const char* entry_point,
    const char* shader_profile,
    unsigned int compile_flags
    );

public :
  virtual ~DirectXShader() {}

  ID3D11ShaderReflection* get_reflector() const {
    return base::scoped_ptr_get(shader_reflector_);
  }

  ID3D10Blob* get_bytecode() const {
    return base::scoped_ptr_get(shader_bytecode_);
  }
};

bool DirectXShader::create_shader_reflection() {
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

bool DirectXShader::compile_shader_to_bytecode(
  const char* file_name,
  const char* entry_point,
  const char* profile,
  unsigned compile_flags
  )
{
  base::scoped_ptr_reset(shader_bytecode_);

  using namespace d3d_details;
  compile_result_t compile_result(compile_shader_from_file(
    file_name, entry_point, profile, compile_flags));
  if (!std::get<0>(compile_result))
    return false;

  base::scoped_ptr_reset(shader_bytecode_, std::get<1>(compile_result));
  return true;
}

class directx_fragment_shader : public DirectXShader {
private :
  NO_CC_ASSIGN(directx_fragment_shader);

  base::scoped_ptr<ID3D11PixelShader, com_ptr>      shader_handle_;
  static const char* const kFragmentProfileNames[];

public :
  enum fragment_profile {
    fragment_profile_4_0,
    fragment_profile_5_0
  };

  ID3D11PixelShader* get_shader_handle() const {
    return base::scoped_ptr_get(shader_handle_);
  }

  bool compile_from_file(
    const char* file_name, 
    const char* entry_point, 
    int profile, 
    unsigned debug_flags
    );

  void bind_to_pipeline(game::renderer* renderer);
};

const char* const directx_fragment_shader::kFragmentProfileNames[] = {
  "ps_4_0",
  "ps_5_0"
};

bool directx_fragment_shader::compile_from_file(
  const char* file_name, 
  const char* entry_point, 
  int profile, 
  unsigned compile_flags
  )
{
  base::scoped_ptr_reset(shader_handle_);
  base::scoped_ptr_reset(shader_reflector_);
  base::scoped_ptr_reset(shader_bytecode_);

  if (!DirectXShader::compile_shader_to_bytecode(
        file_name, entry_point, 
        directx_fragment_shader::kFragmentProfileNames[profile],
        compile_flags) ||
        !DirectXShader::create_shader_reflection()) {
    return false;
  }

  HRESULT ret_code;
  base::scoped_ptr<ID3D11PixelShader, com_ptr> ptr_pixelshader;
  CHECK_D3D(
    &ret_code,
    game::renderer_instance_t::get()->get_device()->CreatePixelShader(
      shader_bytecode_->GetBufferPointer(),
      shader_bytecode_->GetBufferSize(),
      nullptr,
      base::scoped_ptr_get_ptr_ptr(shader_handle_)
    ));

  return ret_code == S_OK;
}

void directx_fragment_shader::bind_to_pipeline(game::renderer* renderer) {
  renderer->get_device_context()->PSSetShader(base::scoped_ptr_get(shader_handle_), nullptr, 0);
}

class directx_vertex_shader : public DirectXShader {
private :
  NO_CC_ASSIGN(directx_vertex_shader);

  base::scoped_ptr<ID3D11VertexShader, com_ptr>     shader_handle_;
  static const char* const kShaderProfileNames[];
public :
  enum vertex_profile {
    vertex_profile_4_0,
    vertex_profile_5_0
  };

  bool compile_from_file(
    const char* file_name, 
    const char* entry_point, 
    int profile, 
    unsigned compile_flags
    );

  ID3D11VertexShader* get_shader_handle() const {
    return base::scoped_ptr_get(shader_handle_);
  }
};

const char* const directx_vertex_shader::kShaderProfileNames[] = {
  "vs_4_0",
  "vs_5_0"
};

bool directx_vertex_shader::compile_from_file(
  const char* file_name, 
  const char* entry_point, 
  int profile, 
  unsigned compile_flags
  )
{
  base::scoped_ptr_reset(shader_handle_);
  base::scoped_ptr_reset(shader_bytecode_);
  base::scoped_ptr_reset(shader_reflector_);

  const bool succeeded = DirectXShader::compile_shader_to_bytecode(
      file_name, entry_point, 
      directx_vertex_shader::kShaderProfileNames[profile],
      compile_flags) &&
      DirectXShader::create_shader_reflection();

  if (!succeeded)
    return false;

  HRESULT ret_code;
  CHECK_D3D(
    &ret_code,
    game::renderer_instance_t::get()->get_device()->CreateVertexShader(
      shader_bytecode_->GetBufferPointer(),
      shader_bytecode_->GetBufferSize(),
      nullptr,
      base::scoped_ptr_get_ptr_ptr(shader_handle_)
    ));

  return ret_code == S_OK;
}

template<typename T>
class uniform_buffer_directx {
private :
  T                                         uniform_data_;
  base::scoped_ptr<ID3D11Buffer, com_ptr>   buff_ptr_;

public :
  uniform_buffer_directx() {}

  uniform_buffer_directx(const T& uniform_data) 
    : uniform_data_(uniform_data) {}

  T& get_buffered_data() {
    return uniform_data_;
  }

  const T& get_buffered_data() {
    return uniform_data_;
  }

  bool initialize();

  void sync_with_gpu();

  ID3D11Buffer* get_buffer_handle() const {
    return base::scoped_ptr_get(buff_ptr_);
  }
};


#if defined(OPENGL_BUILDPASS)

template<typename T>
struct ubuff_rebind {
  typedef uniform_buffer_opengl<T> uniform_buff_t;
};

#else


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

struct test::simple_object::implementation_details {
private :
    NO_CC_ASSIGN(implementation_details);

    struct uniforms_t {
      gfx::matrix_4X4F viewMatrix;
      gfx::matrix_4X4F projectionMatrix;
    };

public :
    base::scoped_ptr<ID3D11InputLayout, com_ptr>    vertex_layout_;
    base::scoped_ptr<ID3D11Buffer, com_ptr>         vertex_buffer_;
    base::scoped_ptr<ID3D11Buffer, com_ptr>         index_buffer_;
    //base::scoped_ptr<ID3D11VertexShader, com_ptr>   vertex_shader_;
    directx_vertex_shader                           vertex_shader_;
    directx_fragment_shader                         fragment_shader_;
    unsigned int                                    num_indices_;
    gfx::matrix_4X4F                                model2world_;
    ubuff_rebind<uniforms_t>::uniform_buff_t        uniforms_buffer_;

    implementation_details() 
        : num_indices_(0), 
          model2world_(gfx::matrix_4X4F::identity),
          uniforms_buffer_() {}
};

test::simple_object::simple_object() : impl_(new implementation_details) {}

test::simple_object::~simple_object() {}

void test::simple_object::initialize() {
    const char* kVertexShaderFile = "C:\\temp\\shader_cache\\p1\\vertex_shader.hlsl";

    const UINT kShaderCompileFlags = 
      D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS |
      D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_DEBUG |
      D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    if (!impl_->vertex_shader_.compile_from_file(
      kVertexShaderFile, "vs_main", directx_vertex_shader::vertex_profile_5_0,
      kShaderCompileFlags
      )) {
      NOT_REACHED_MSG(L"Failed to compile vertex shader");
      return;
    }

    D3D11_INPUT_ELEMENT_DESC vertex_description[] = {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
        },
        {
            "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
        }
    };

    HRESULT ret_code;
    CHECK_D3D(
        &ret_code,
        game::renderer_instance_t::get()->get_device()->CreateInputLayout(
            vertex_description, _countof(vertex_description),
            shader_bytecode->GetBufferPointer(),
            shader_bytecode->GetBufferSize(),
            base::scoped_ptr_get_ptr_ptr(impl_->vertex_layout_)
            ));

    if (FAILED(ret_code)) {
        NOT_REACHED();
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

    D3D11_BUFFER_DESC buffer_description = {
        model_vertices.size() * sizeof(model_vertices[0]),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0, // no CPU access necessary
        0,
        sizeof(model_vertices[0])
    };

    D3D11_SUBRESOURCE_DATA buffer_initdata = { &model_vertices[0], 0, 0 };

    CHECK_D3D(
        &ret_code,
        game::renderer_instance_t::get()->get_device()->CreateBuffer(
            &buffer_description, &buffer_initdata, 
            base::scoped_ptr_get_ptr_ptr(impl_->vertex_buffer_)));
    if (FAILED(ret_code)) {
        NOT_REACHED();
        return;
    }

    buffer_description.ByteWidth = model_loader.getIndexCount() * sizeof(DWORD);
    buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_description.StructureByteStride = sizeof(DWORD);
    buffer_initdata.pSysMem = model_loader.getVertexListPointer();

    CHECK_D3D(
        &ret_code,
        game::renderer_instance_t::get()->get_device()->CreateBuffer(
            &buffer_description, &buffer_initdata, 
            base::scoped_ptr_get_ptr_ptr(impl_->index_buffer_)));
    if (FAILED(ret_code)) {
        NOT_REACHED();
        return;
    }

    const char* const kPixelShaderFile = "C:\\temp\\shader_cache\\p2\\pixel_shader.hlsl";
    compile_res = compile_shader_from_file(kPixelShaderFile, "ps_main", 
                                           "ps_5_0", kCompileFlags);
    if (!std::get<0>(compile_res)) {
        NOT_REACHED();
        return;
    }

    base::scoped_ptr_reset(shader_bytecode, std::get<1>(compile_res));
    CHECK_D3D(
        &ret_code,
        game::renderer_instance_t::get()->get_device()->CreatePixelShader(
            shader_bytecode->GetBufferPointer(), 
            shader_bytecode->GetBufferSize(),
            nullptr,
            base::scoped_ptr_get_ptr_ptr(impl_->pixel_shader_)
            ));
    if (FAILED(ret_code)) {
        NOT_REACHED();
        return;
    }
}

void test::simple_object::draw() {
    ID3D11DeviceContext* context = game::renderer_instance_t::get()->get_device_context();

    context->IASetInputLayout(base::scoped_ptr_get(impl_->vertex_layout_));
    ID3D11Buffer* buffers[] = { base::scoped_ptr_get(impl_->vertex_buffer_) };
    UINT strides = sizeof(vertex_t);
    UINT offsets = 0;
    context->IASetVertexBuffers(0, _countof(buffers), buffers, 
                                &strides, &offsets);
    context->IASetIndexBuffer(base::scoped_ptr_get(impl_->index_buffer_), 
                              DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(base::scoped_ptr_get(impl_->vertex_shader_), nullptr, 0);
    context->PSSetShader(base::scoped_ptr_get(impl_->pixel_shader_), nullptr, 0);
    context->GSSetShader(nullptr, nullptr, 0);

    context->DrawIndexed(impl_->num_indices_, 0, 0);
}