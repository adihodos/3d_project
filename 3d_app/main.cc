#include "pch_hdr.h"
#include "atexit_manager.h"
#include "scoped_ptr.h"
#include "win32_utils.h"
#include "window.h"

int WINAPI wWinMain(
    HINSTANCE inst,
    HINSTANCE,
    LPWSTR,
    int
    )
{
    {
        base::atexit_manager atexit_manager_object;
        game::game_window main_window(inst);
        if (!main_window.create(1024, 768)) {
            win32::utils::varargs_msgbox(
                MB_ICONERROR | MB_OK, L"Fatal error",
                L"Failed to initialize window");
            return EXIT_FAILURE;
        }
        game::pump_message_loop();
    }    
    return EXIT_SUCCESS;
}