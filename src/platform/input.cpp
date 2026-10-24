#include "input.hpp"
namespace
{
LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM key, LPARAM detail)
{
    auto *input = reinterpret_cast<WindowInput *>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE)
    {
        auto *creation = reinterpret_cast<CREATESTRUCTW *>(detail);
        input = static_cast<WindowInput *>(creation->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(input));
    }
    if (!input)
        return DefWindowProcW(window, message, key, detail);
    switch (message)
    {
    case WM_DESTROY:
        input->quit = true;
        PostQuitMessage(0);
        return 0;
    case WM_KILLFOCUS:
        input->active = false;
        input->lostFocus = true;
        input->held.fill(false);
        input->pressed.fill(false);
        return 0;
    case WM_SETFOCUS:
        input->active = true;
        return 0;
    case WM_KEYDOWN:
        if (key < 256)
        {
            if (!input->held[key])
                input->pressed[key] = true;
            input->held[key] = true;
        }
        return 0;
    case WM_KEYUP:
        if (key < 256)
            input->held[key] = false;
        return 0;
    default:
        return DefWindowProcW(window, message, key, detail);
    }
}
} // namespace
HWND create_window(HINSTANCE instance, WindowInput &input)
{
    WNDCLASSW type{};
    type.lpfnWndProc = window_proc;
    type.hInstance = instance;
    type.lpszClassName = L"SlideReadableWindow";
    type.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&type);
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT bounds{0, 0, 1280, 720};
    AdjustWindowRect(&bounds, style, FALSE);
    HWND window = CreateWindowW(type.lpszClassName, L"SLIDE - Wrong group chat", style,
                                CW_USEDEFAULT, CW_USEDEFAULT, bounds.right - bounds.left,
                                bounds.bottom - bounds.top, nullptr, nullptr, instance, &input);
    if (!window)
        throw std::runtime_error("Could not create window");
    return window;
}
void WindowInput::poll()
{
    MSG message;
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
            quit = true;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}
void WindowInput::finish_frame()
{
    pressed.fill(false);
    lostFocus = false;
}
slide::Input WindowInput::movement(bool jump) const
{
    slide::Input result;
    result.x = double(held['D'] || held[VK_RIGHT]) - double(held['A'] || held[VK_LEFT]);
    result.z = double(held['W'] || held[VK_UP]) - double(held['S'] || held[VK_DOWN]);
    result.brake = held[VK_SHIFT];
    result.jump = jump;
    return result;
}
