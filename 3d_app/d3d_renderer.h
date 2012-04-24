#pragma once

#include "lazy_singleton.h"
#include "misc.h"
#include "scoped_ptr.h"

namespace outer_limits {
  class directx_vertexbuffer;
  class directx_index_buffer;
  class directx_input_layout;
}

namespace game {


class renderer : public base::singleton_life_traits<renderer>  {
private :
    NO_CC_ASSIGN(renderer);
    friend class base::singleton_life_traits<renderer>;

    struct implementation_details;
    base::scoped_ptr<implementation_details> impl_;

    renderer();

    ~renderer();
public :
    enum clear_flags {
        clear_depth = 1U << 0,
        clear_stencil = 1U << 1
    };

    enum presentation_flags {
        pf_present_frame,
        pf_present_test_frame
    };

    enum presentation_result {
        present_ok,
        present_window_occluded,
        present_error
    };

    bool initialize(HWND window, int width, int height, bool fullscreen = false);

    bool resize_buffers(int width, int height);

    int get_width() const;

    int get_height() const;

    void clear_backbuffer(const float color[4]);

    void clear_depth_stencil(
        unsigned int clr_flags = clear_depth | clear_stencil, 
        float depth = 1.0f, 
        unsigned char stencil = 0xff
        );

    presentation_result present(presentation_flags fp = pf_present_frame);

    ID3D11Device* get_device() const;

    ID3D11DeviceContext* get_device_context() const;

    void ia_stage_set_vertex_buffers(
      unsigned start_slot,
      unsigned buffer_count,
      const outer_limits::directx_vertexbuffer* buffers,
      const unsigned* strides,
      const unsigned* offsets = nullptr
      );

    void ia_stage_set_index_buffer(
      const outer_limits::directx_index_buffer* index_buffer,
      unsigned offset = 0U
      );

    void ia_stage_set_input_layout(
      const outer_limits::directx_input_layout*
      );

    void ia_stage_set_primitive_topology(
      int topology
      );

    void draw(unsigned vertex_count, unsigned offset = 0U);

    void draw_indexed(
      unsigned index_count, 
      unsigned index_offset = 0U, 
      int index_add = 0
      );
};

typedef base::lazy_singleton<renderer> renderer_instance_t;

} // namespace game