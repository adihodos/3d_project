#pragma once

#include <vector>
#include <DXGI.h>
#include <Windows.h>
#include "scoped_ptr.h"
#include "win32_traits.h"

namespace win32 {

namespace utils {

class scoped_dc {
private :
    scoped_dc(const scoped_dc&);
    scoped_dc& operator=(const scoped_dc&);

    HWND    owning_wnd_;
    HDC     devctx_;
    
public :
    scoped_dc(HWND wnd, HDC devctx) 
        : owning_wnd_(wnd), devctx_(devctx) {}

    ~scoped_dc() {
        if (devctx_)
            ::ReleaseDC(owning_wnd_, devctx_);
    }

    HDC get() const {
        return devctx_;
    }

    bool operator!() const {
        return devctx_ == nullptr;
    }

    HDC release() {
        HDC old_ctx = devctx_;
        devctx_ = nullptr;
        return old_ctx;
    }
};

void varargs_msgbox(
    UINT buttons, 
    const wchar_t* title, 
    const wchar_t* fmt, 
    ...
    );

typedef base::scoped_ptr<IDXGIFactory, traits::com_ptr> unique_dxgifactory_handle_t;

unique_dxgifactory_handle_t create_dxgi_factory();

typedef base::scoped_ptr<IDXGIAdapter, traits::com_ptr> unique_adaptor_handle_t;

std::vector<unique_adaptor_handle_t> enum_graphic_adapters(
    IDXGIFactory* factory
    );

typedef base::scoped_ptr<IDXGIOutput, traits::com_ptr> unique_output_handle_t;

std::vector<unique_output_handle_t> enum_adapter_outputs(
    IDXGIAdapter* adapter
    );

std::vector<DXGI_MODE_DESC> enum_output_modes(
    IDXGIOutput* output, 
    DXGI_FORMAT format, 
    UINT enum_flags = 0
    );

} // namespace utils

} // namespace win32