#pragma once

#include <Windows.h>
#include "misc.h"

namespace game {

class game_window {
public :
    struct vars_t {
        HWND        window_handle_;
        HINSTANCE   instance_;
        int         width_;
        int         height_;
        bool        fullscreen_;
        bool        minimized_;
        bool        renderer_initialized_;
    };

private :
    NO_CC_ASSIGN(game_window);
    vars_t              win32_state_;
    static bool         window_class_is_registered_;

    struct create_params_t {
        DWORD   ex_style;
        DWORD   style;
        RECT    dimensions;
    };

    static LRESULT CALLBACK window_procedure_stub(
        HWND window,
        UINT msg,
        WPARAM w_param,
        LPARAM l_param
        );

    bool register_window_class();

    LRESULT real_window_procedure(UINT msg, WPARAM w_param, LPARAM l_param);

    bool real_create(const create_params_t&);

    void wm_paint();

    void handle_wm_size(WPARAM sizing_request, int size_x, int size_y);

public :
    game_window(HINSTANCE instance) {
        memset(&win32_state_, 0, sizeof(win32_state_));
        win32_state_.instance_ = instance;
    }

    bool create(int width, int height);

    bool create_fullscreen();

    const vars_t& get_info() const {
        return win32_state_;
    }
};

extern game_window* main_window;

void pump_message_loop();

} // namespace game