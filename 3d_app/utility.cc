#include "pch_hdr.h"
#include "utility.h"

void utility::string_v_format(
    wchar_t*    dst_str,
    size_t      dst_len,
    const wchar_t* fmt_spec,
    ...
    )
{
    va_list args_ptr;
    va_start(args_ptr, fmt_spec);
    _vsnwprintf_s(dst_str, dst_len, _TRUNCATE, fmt_spec, args_ptr);
    va_end(args_ptr);
}

void utility::output_debug_string(
    const wchar_t* file,
    int line,
    const wchar_t* fmt_spec,
    ...
    )
{
    wchar_t buff_msg[2048];
    _snwprintf_s(buff_msg, _countof(buff_msg), _TRUNCATE, L"\n[File %s, line %d]\n", file, line);
    ::OutputDebugStringW(buff_msg);

    va_list args_ptr;
    va_start(args_ptr, fmt_spec);
    _vsnwprintf_s(buff_msg, _countof(buff_msg), _TRUNCATE, fmt_spec, args_ptr);
    va_end(args_ptr);

    ::OutputDebugStringW(buff_msg);
}

void utility::output_debug_string(
    const char* file,
    int line,
    const char* fmt,
    ...
    )
{
    char buff_msg[2048];
    _snprintf_s(buff_msg, _countof(buff_msg), _TRUNCATE, "\n[File %s, line %d]\n", file, line);
    ::OutputDebugStringA(buff_msg);

    va_list args_ptr;
    va_start(args_ptr, fmt);
    _vsnprintf_s(buff_msg, _countof(buff_msg), _TRUNCATE, fmt, args_ptr);
    va_end(args_ptr);

    ::OutputDebugStringA(buff_msg);
}