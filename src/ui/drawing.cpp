#include "../render/renderer.hpp"
#include <cstring>
#include <iomanip>
#include <sstream>
using namespace slide;
void Renderer::rect(int x, int y, int w, int h, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    RECT r{x, y, x + w, y + h};
    FillRect(dc, &r, brush);
    DeleteObject(brush);
}

void Renderer::text(int x, int y, int size, const std::wstring &s, COLORREF color, bool bold)
{
    HFONT font = CreateFontW(-size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    auto old = SelectObject(dc, font);
    SetTextColor(dc, color);
    TextOutW(dc, x, y, s.c_str(), (int)s.size());
    SelectObject(dc, old);
    DeleteObject(font);
}

void Renderer::wrapped(int x, int y, int width, int height, int size, const std::wstring &str,
                       COLORREF color, bool bold)
{
    HFONT font = CreateFontW(-size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    auto old = SelectObject(dc, font);
    SetTextColor(dc, color);
    RECT area{x, y, x + width, y + height};
    DrawTextW(dc, str.c_str(), (int)str.size(), &area, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
    SelectObject(dc, old);
    DeleteObject(font);
}

std::wstring Renderer::wide(const std::string &value)
{
    if (value.empty())
        return {};
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), int(value.size()),
                                    nullptr, 0);
    if (!count)
        throw std::runtime_error("Text is not valid UTF-8");
    std::wstring result(size_t(count), L' ');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), int(value.size()), result.data(), count);
    return result;
}

std::wstring Renderer::time(float sec)
{
    std::wostringstream s;
    s << std::fixed << std::setprecision(2) << sec << L"s";
    return s.str();
}