#include "pch_hdr.h"
#include "d3d_renderer.h"
#include "scene.h"
#include "utility.h"
#include "win32_utils.h"
#include "window.h"

namespace {
    const wchar_t* const kWindowClassName = L"D3DWindow_##__";
}

bool game::game_window::window_class_is_registered_ = false;

game::game_window* game::main_window;

LRESULT CALLBACK game::game_window::window_procedure_stub(
    HWND window,
    UINT msg,
    WPARAM w_param,
    LPARAM l_param
    )
{
    if (msg == WM_NCCREATE) {
        const CREATESTRUCTW* csw = reinterpret_cast<CREATESTRUCTW*>(l_param);
        ::SetWindowLongPtr(window, GWLP_USERDATA, 
                           reinterpret_cast<LONG_PTR>(csw->lpCreateParams));
        main_window = static_cast<game_window*>(csw->lpCreateParams);
        return true;
    }

    game_window* wptr = reinterpret_cast<game_window*>(
        ::GetWindowLongPtrW(window, GWLP_USERDATA));
    return wptr ? wptr->real_window_procedure(msg, w_param, l_param) :
        ::DefWindowProcW(window, msg, l_param, w_param);
}

bool game::game_window::register_window_class() {
    if (game_window::window_class_is_registered_)
        return true;

    WNDCLASSEXW class_data = {
        sizeof(WNDCLASSEXW),
        0,
        game_window::window_procedure_stub,
        0, 0,
        win32_state_.instance_,
        ::LoadIconW(nullptr, IDI_APPLICATION),
        ::LoadCursorW(nullptr, IDC_ARROW),
        reinterpret_cast<HBRUSH>(COLOR_GRAYTEXT),
        nullptr,
        kWindowClassName,
        nullptr
    };

    BOOL ret_code;
    WIN32_CHK_FNCALL(&ret_code, ::RegisterClassExW(&class_data));
    if (ret_code) {
        game_window::window_class_is_registered_ = true;
        return true;
    }
    return false;
}

LRESULT game::game_window::real_window_procedure(
    UINT msg,
    WPARAM w_param,
    LPARAM l_param
    )
{
    switch (msg) {
    case WM_CLOSE :
        ::DestroyWindow(win32_state_.window_handle_);
        return 0L;
        break;

    case WM_DESTROY :
        ::PostQuitMessage(0);
        return 0L;
        break;

    case WM_PAINT :
        return 0L;
        break;

    case WM_SIZE :
        handle_wm_size(w_param, LOWORD(l_param), HIWORD(l_param));
        return 0L;
        break;

    default :
        break;
    }

    return ::DefWindowProcW(win32_state_.window_handle_, msg, w_param, l_param);
}

void game::game_window::handle_wm_size(
    WPARAM sizing_request,
    int size_x,
    int size_y
    )
{
    OUTPUT_DBG_MSGA("WM_SIZE, sizing type [%d], (x, y) -> (%d, %d)",
                    sizing_request, size_x, size_y);

    win32_state_.width_ = size_x;
    win32_state_.height_ = size_y;

    if (!win32_state_.renderer_initialized_)
        return;

    switch (sizing_request) {
    case SIZE_MAXIMIZED :
        win32_state_.fullscreen_ = true;
        renderer_instance_t::get()->resize_buffers(size_x, size_y);
        break;

    case SIZE_MINIMIZED :
        win32_state_.minimized_ = true;
        break;

    case SIZE_RESTORED :
        win32_state_.minimized_ = false;
        renderer_instance_t::get()->resize_buffers(size_x, size_y);
        break;

    default :    
        break;
    }
}

bool game::game_window::create(
    int width,
    int height
    )
{
    create_params_t create_info = {
        WS_EX_OVERLAPPEDWINDOW,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        { 0, 0, width, height }
    };

    ::AdjustWindowRectEx(&create_info.dimensions, create_info.style, false,
                         create_info.ex_style);

    return real_create(create_info);
}

bool game::game_window::create_fullscreen() {
    DEVMODEW res_info;
    ::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &res_info);
    create_params_t create_info = {
        0,
        WS_POPUP | WS_VISIBLE,
        { 0, 0, res_info.dmPelsWidth, res_info.dmPelsHeight }
    };

    win32_state_.fullscreen_ = true;
    return real_create(create_info);
}

bool game::game_window::real_create(
    const game::game_window::create_params_t& params
    )
{
    if (!register_window_class())
        return false;

    win32_state_.width_ = params.dimensions.right - params.dimensions.left;
    win32_state_.height_ = params.dimensions.bottom - params.dimensions.top;

    win32_state_.window_handle_ = ::CreateWindowExW(
        params.ex_style, kWindowClassName, L"D3D Window!",
        params.style, 
        0, 0,
        win32_state_.width_,
        win32_state_.height_,
        nullptr, nullptr, win32_state_.instance_, this
        );

    if (!win32_state_.window_handle_)
        return false;

    ::ShowWindow(win32_state_.window_handle_, SW_SHOWNORMAL);
    ::UpdateWindow(win32_state_.window_handle_);

    win32_state_.renderer_initialized_ = 
        renderer_instance_t::get()->initialize(
            win32_state_.window_handle_, 
            win32_state_.width_,
            win32_state_.height_,
            win32_state_.fullscreen_
            );

    return win32_state_.renderer_initialized_;
}

void game::pump_message_loop() {
    MSG message;
    bool occluded_window_flag = false;

    for (bool quit_flag = false; !quit_flag;) {
        if (::PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE)) {
            BOOL ret_code = ::GetMessageW(&message, nullptr, 0, 0);
            if (ret_code == -1) {
                OUTPUT_DBG_MSGW(L"GetMessage() error %d", ::GetLastError());
                return;
            }

            if (ret_code == 0)
                return;

            ::TranslateMessage(&message);
            ::DispatchMessageW(&message);
        }

        float clear_color[4] = { 0.64f, 0.128f, 0.92f, 1.0f };
        renderer_instance_t::get()->clear_backbuffer(clear_color);
        renderer_instance_t::get()->clear_depth_stencil();

        renderer::presentation_flags pflags = 
            occluded_window_flag ? renderer::pf_present_test_frame :
                                   renderer::pf_present_frame;

        int ret_code = renderer_instance_t::get()->present(pflags);
        switch (ret_code) {
        case renderer::present_error :
            quit_flag = true;
            break;

        case renderer::present_window_occluded :
            occluded_window_flag = true;
            ::Sleep(50);
            break;

        default :
            occluded_window_flag = false;
            break;
        }
    }
}