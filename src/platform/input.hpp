#pragma once
#include "windows.hpp"
#include "../game/game.hpp"
#include <array>
struct WindowInput
{
    std::array<bool, 256> held{};
    std::array<bool, 256> pressed{};
    bool active = true;
    bool lostFocus = false;
    bool quit = false;
    void poll();
    void finish_frame();
    slide::Input movement(bool jump) const;
};
HWND create_window(HINSTANCE instance, WindowInput &input);
