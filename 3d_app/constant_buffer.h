#pragma once

#include <d3d11.h>
#include "misc.h"
#include "scoped_ptr.h"
#include "win32_traits.h"

namespace base {

class constant_buffer_base {
public :
    virtual ~constant_buffer_base() {}

    virtual void create() = 0;

    virtual void update() = 0;

    virtual void bind() = 0;
};

class gpu_shader;
class renderer;

template<typename T>
class constant_buffer : public constant_buffer_base {
    T   userdata_;
    ID3D11Buffer* native_buff_handle_;
    ID3D11DeviceContext* device_;
public :
    void create() {
    }

    T& get_userdata() {
        return userdata_;
    }

    void update() {
        D3D11_MAPPED_SUBRESOURCE msr;
        device_->Map(native_buff_handle_, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        memcpy(msr.pData, &userdata_, sizeof(userdata_));
        device_->Unmap(native_buff_handle_, 0);
    }
};

}