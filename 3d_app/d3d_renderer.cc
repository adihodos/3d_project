#include "pch_hdr.h"
#include "d3d_renderer.h"
#include "scoped_ptr.h"
#include "utility.h"
#include "win32_traits.h"
#include "win32_utils.h"

using namespace win32::traits;
using namespace base;

struct game::renderer::implementation_details {
private :
    NO_CC_ASSIGN(implementation_details);

public :
    scoped_ptr<IDXGIFactory, com_ptr>           dxgi_factory_;
    scoped_ptr<IDXGISwapChain, com_ptr>         dxgi_swapchain_;
    scoped_ptr<ID3D11Device, com_ptr>           d3d11device_;
    scoped_ptr<ID3D11DeviceContext, com_ptr>    d3d11device_ctx_;
    scoped_ptr<ID3D11RenderTargetView, com_ptr> rendertargetview_;
    scoped_ptr<ID3D11Texture2D, com_ptr>        depthstencil_texture_;
    scoped_ptr<ID3D11DepthStencilView, com_ptr> depthstencilview_;
    DXGI_MODE_DESC                              mode_info_;
    int                                         width_;
    int                                         height_;

    implementation_details() : width_(0), height_(0) {}

    void release_resources_before_buffer_resize();

    bool initialize(HWND window, int width, int height, bool fullscreen);

    bool create_render_target_view();

    bool create_depth_stencil_buffer_and_views();

    bool bind_to_output_merger_stage();
};

void game::renderer::implementation_details::release_resources_before_buffer_resize() {
    d3d11device_ctx_->OMSetRenderTargets(0, nullptr, nullptr);
    scoped_ptr_reset(depthstencilview_);
    scoped_ptr_reset(depthstencil_texture_);
    scoped_ptr_reset(rendertargetview_);
}

bool game::renderer::implementation_details::initialize(
    HWND window,
    int width, 
    int height,
    bool fullscreen
    )
{
    assert(!dxgi_swapchain_);
    assert(!d3d11device_);
    assert(!d3d11device_ctx_);

    using namespace win32::utils;

    unique_dxgifactory_handle_t dxgiFactory(create_dxgi_factory());
    if (!dxgiFactory)
        return false;

    std::vector<unique_adaptor_handle_t> graphicAdapters(
        enum_graphic_adapters(scoped_ptr_get(dxgiFactory)));
    if (graphicAdapters.empty())
        return false;

    bool modeFound = false;
    size_t i = 0;
    for (i = 0; i < graphicAdapters.size() && !modeFound; ++i) {
        std::vector<unique_output_handle_t> adapterOutputs(
            enum_adapter_outputs(scoped_ptr_get(graphicAdapters[i])));
        if (adapterOutputs.empty())
            continue;

        for (size_t j = 0; j < adapterOutputs.size() && !modeFound; ++j) {
            auto modes_list = enum_output_modes(scoped_ptr_get(adapterOutputs[j]), 
                                                DXGI_FORMAT_R8G8B8A8_UNORM);
            DXGI_MODE_DESC req_mode = {
                width, 
                height, 
                { 0, 0 }, 
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
                DXGI_MODE_SCALING_UNSPECIFIED
            };

            modeFound = adapterOutputs[j]->FindClosestMatchingMode(
                &req_mode, &mode_info_, nullptr) == S_OK;
        }
    }

    if (!modeFound)
        return false;

    DXGI_SWAP_CHAIN_DESC swp_chain_desc = {
        mode_info_,
        { 1, 0 },
        DXGI_USAGE_RENDER_TARGET_OUTPUT,
        1,
        window,
        !fullscreen,
        DXGI_SWAP_EFFECT_DISCARD,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
    };

    UINT creation_flags = D3D11_CREATE_DEVICE_DEBUG;
    HRESULT ret_code;
    CHECK_D3D(
        &ret_code,
        ::D3D11CreateDeviceAndSwapChain(
            scoped_ptr_get(graphicAdapters[i - 1]),
            D3D_DRIVER_TYPE_UNKNOWN, 
            nullptr,
            creation_flags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &swp_chain_desc,
            scoped_ptr_get_ptr_ptr(dxgi_swapchain_),
            scoped_ptr_get_ptr_ptr(d3d11device_),
            nullptr,
            scoped_ptr_get_ptr_ptr(d3d11device_ctx_)
            ));

    if (FAILED(ret_code))
        return false;

    return create_render_target_view() 
           && create_depth_stencil_buffer_and_views()
           && bind_to_output_merger_stage();
}

bool game::renderer::implementation_details::create_render_target_view() {
    assert(!rendertargetview_);
    scoped_ptr<ID3D11Texture2D, com_ptr> texture_resource;
    HRESULT ret_code;
    CHECK_D3D(
		&ret_code,
        dxgi_swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), 
			reinterpret_cast<void**>(scoped_ptr_get_ptr_ptr(texture_resource))));
	CHECK_D3D(
		&ret_code,
		d3d11device_->CreateRenderTargetView(
			scoped_ptr_get(texture_resource),
			nullptr,
			scoped_ptr_get_ptr_ptr(rendertargetview_)));

	return ret_code == S_OK;
}

bool 
game::renderer::implementation_details::create_depth_stencil_buffer_and_views() {
    assert(!depthstencilview_);
    assert(!depthstencil_texture_);

    D3D11_TEXTURE2D_DESC texture_desc = {
        mode_info_.Width,
        mode_info_.Height,
        0,
        1,
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
        { 1, 0 },
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_DEPTH_STENCIL,
        0,
        0
    };

    HRESULT ret_code;
    CHECK_D3D(
        &ret_code,
        d3d11device_->CreateTexture2D(
            &texture_desc, nullptr, 
            scoped_ptr_get_ptr_ptr(depthstencil_texture_)));

    if (FAILED(ret_code))
        return false;

    CHECK_D3D(
        &ret_code,
        d3d11device_->CreateDepthStencilView(
            scoped_ptr_get(depthstencil_texture_),
            nullptr,
            scoped_ptr_get_ptr_ptr(depthstencilview_)));

    return ret_code == S_OK;
}

bool game::renderer::implementation_details::bind_to_output_merger_stage() {
    ID3D11RenderTargetView* render_target_views[] = { 
        scoped_ptr_get(rendertargetview_) 
    };
    d3d11device_ctx_->OMSetRenderTargets(_countof(render_target_views), 
                                         render_target_views, 
                                         scoped_ptr_get(depthstencilview_));

    D3D11_VIEWPORT view_port = {
        0, 0,
        static_cast<float>(width_), 
        static_cast<float>(height_),
        0.0f, 1.0f
    };
    d3d11device_ctx_->RSSetViewports(1, &view_port);
    return true;
}

game::renderer::renderer()
    : impl_(new implementation_details) {}

game::renderer::~renderer() {
    if (!!impl_->dxgi_swapchain_)
        impl_->dxgi_swapchain_->SetFullscreenState(false, nullptr);
}

bool game::renderer::initialize(
    HWND window, 
    int width, 
    int height, 
    bool fullscreen)
{
    return impl_->initialize(window, width, height, fullscreen);
}

bool game::renderer::resize_buffers(int width, int height) {
    impl_->release_resources_before_buffer_resize();
    impl_->mode_info_.Width = static_cast<UINT>(width);
    impl_->mode_info_.Height = static_cast<UINT>(height);
    HRESULT ret_code;
    CHECK_D3D(
        &ret_code,
        impl_->dxgi_swapchain_->ResizeBuffers(
            0, width, height, DXGI_FORMAT_UNKNOWN, 0));
    if (FAILED(ret_code))
        return false;

    return impl_->create_render_target_view() 
           && impl_->create_depth_stencil_buffer_and_views()
           && impl_->bind_to_output_merger_stage();
}

int game::renderer::get_width() const {
    return impl_->width_;
}

int game::renderer::get_height() const {
    return impl_->height_;
}

void game::renderer::clear_backbuffer(
    const float color[4]
    )
{
    impl_->d3d11device_ctx_->ClearRenderTargetView(
        scoped_ptr_get(impl_->rendertargetview_), color); 
}

void game::renderer::clear_depth_stencil(
    unsigned int cf,
    float depth, 
    unsigned char stencil
    )
{
    unsigned d3d_clear_flag = D3D11_CLEAR_FLAG(0);
    if (cf & clear_depth)
        d3d_clear_flag |= D3D11_CLEAR_DEPTH;

    if (cf & clear_stencil)
        d3d_clear_flag |= D3D11_CLEAR_STENCIL;

    impl_->d3d11device_ctx_->ClearDepthStencilView(
        scoped_ptr_get(impl_->depthstencilview_),
        d3d_clear_flag, depth, stencil
        );
}

game::renderer::presentation_result
game::renderer::present(game::renderer::presentation_flags pf) {
    HRESULT ret_code = impl_->dxgi_swapchain_->Present(0, pf);
    switch (ret_code) {
    case S_OK :
        return present_ok;
        break;

    case DXGI_STATUS_OCCLUDED :
        return present_window_occluded;
        break;

    default :
        break; 
    }

    return present_error;
}

ID3D11Device* game::renderer::get_device() const {
    return base::scoped_ptr_get(impl_->d3d11device_);
}

ID3D11DeviceContext* game::renderer::get_device_context() const {
    return base::scoped_ptr_get(impl_->d3d11device_ctx_);
}