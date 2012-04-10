#pragma once

#if defined(OPENGL_BUILDPASS)
#else

#include "uniform_buffer_directx.h"

namespace renderer {

template<typename T>
struct ubuff_rebind {
  typedef uniform_buffer_directx<T> uniform_buffer_t;
};

} // namespace renderer

#endif