#pragma once

#include <Windows.h>

namespace win32 {

namespace traits {

struct device_context {
    static HDC null_resource() {
        return nullptr;
    }

    static void dispose(HDC devctx) {
        if (devctx)
            ::DeleteDC(devctx);
    }
};

struct file_deleter {
    void operator()(HANDLE fptr) const {
        if (fptr != INVALID_HANDLE_VALUE)
            ::CloseHandle(fptr);
    }
};

template<typename ComPtr>
struct com_ptr {
    static void dispose(ComPtr* ptr) {
        if (ptr)
            ptr->Release();
    }
};

} // namespace traits
} // namespace win32    