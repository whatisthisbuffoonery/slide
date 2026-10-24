#include "../render/renderer.hpp"
using namespace slide;
void Renderer::makeAtlas(const Course &c)
{
    HDC saved = dc;
    HDC canvas = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 512;
    bi.bmiHeader.biHeight = -1024;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    void *pixels = nullptr;
    HBITMAP dib = CreateDIBSection(canvas, &bi, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!canvas || !dib)
    {
        if (dib)
            DeleteObject(dib);
        if (canvas)
            DeleteDC(canvas);
        throw std::runtime_error("Cannot create message artwork");
    }
    auto old = SelectObject(canvas, dib);
    dc = canvas;
    SetBkMode(dc, TRANSPARENT);
    rect(0, 0, 512, 1024, RGB(247, 226, 233));
    for (int i = 0; i < (int)c.hazards.size(); ++i)
    {
        const auto &m = c.messages[c.hazards[i].message];
        int y = i * 256;
        rect(0, y, 512, 48, RGB(116, 32, 63));
        text(20, y + 10, 25, L"TO: " + wide(m.actual), RGB(255, 235, 242), true);
        wrapped(22, y + 65, 468, 115, 35, wide(m.body), RGB(40, 18, 33), true);
        text(22, y + 214, 21, L"Oops. Meant for " + wide(m.intended), RGB(117, 42, 68));
        rect(0, y + 251, 512, 5, RGB(242, 108, 146));
    }
    GdiFlush();
    context->UpdateSubresource(atlas.p, 0, nullptr, pixels, 512 * 4, 0);
    dc = saved;
    SelectObject(canvas, old);
    DeleteObject(dib);
    DeleteDC(canvas);
}
