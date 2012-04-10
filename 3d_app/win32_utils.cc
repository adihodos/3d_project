#include "pch_hdr.h"
#include "utility.h"
#include "win32_utils.h"

void win32::utils::varargs_msgbox(
    UINT buttons,
    const wchar_t* title,
    const wchar_t* fmt,
    ...
    )
{
    va_list args_ptr;
    va_start(args_ptr, fmt);

    wchar_t buff_msg[2048];
    ::_vsnwprintf_s(buff_msg, _countof(buff_msg), _TRUNCATE, fmt, args_ptr);
    va_end(args_ptr);
    ::MessageBoxW(nullptr, buff_msg, title, buttons);
}

base::scoped_ptr<IDXGIFactory, win32::traits::com_ptr>
win32::utils::create_dxgi_factory() {
    base::scoped_ptr<IDXGIFactory, traits::com_ptr> factory;
    HRESULT ret_code = ::CreateDXGIFactory(
        __uuidof(IDXGIFactory),
        reinterpret_cast<void**>(base::scoped_ptr_get_ptr_ptr(factory)));
    return factory;
}

std::vector<win32::utils::unique_adaptor_handle_t>
win32::utils::enum_graphic_adapters(IDXGIFactory* factory) {
    std::vector<win32::utils::unique_adaptor_handle_t> adapters;
    UINT current_index = 0;
    IDXGIAdapter* current_adapter;
    while (factory->EnumAdapters(current_index++, &current_adapter) == S_OK)
        adapters.push_back(unique_adaptor_handle_t(current_adapter));

    return adapters;
}

std::vector<win32::utils::unique_output_handle_t>
win32::utils::enum_adapter_outputs(IDXGIAdapter* adapter) {
    std::vector<unique_output_handle_t> outputs;
    UINT current_index = 0;
    IDXGIOutput* current_output;
    while (adapter->EnumOutputs(current_index++, &current_output) == S_OK)
        outputs.push_back(unique_output_handle_t(current_output));
    return outputs;
}

std::vector<DXGI_MODE_DESC>
win32::utils::enum_output_modes(
    IDXGIOutput* output, 
    DXGI_FORMAT fmt,
    UINT enum_flags /* = 0 */
    ) {
    std::vector<DXGI_MODE_DESC> modes_list;

    UINT item_count = 0;
    output->GetDisplayModeList(fmt, enum_flags, &item_count, nullptr);
    modes_list.resize(item_count);
    output->GetDisplayModeList(fmt, enum_flags, &item_count, &modes_list[0]);
    return modes_list;
}