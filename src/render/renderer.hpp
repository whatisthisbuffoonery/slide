#pragma once
#include "../platform/windows.hpp"
#include "../game/game.hpp"
#include "mesh.hpp"

// D3D11 is isolated here. Game and content modules contain no Windows types.
constexpr int W = 960;
constexpr int H = 540;
struct Frame
{
    slide::F4 eye, forward, right, up, ball, info, markers[4], hazards[4];
};
static_assert(sizeof(Frame) == 224, "Match the HLSL constant-buffer layout");
struct Renderer
{
    Com<ID3D11Device> device;
    Com<ID3D11DeviceContext> context;
    Com<IDXGISwapChain> swap;
    Com<ID3D11RenderTargetView> target;
    Com<ID3D11ComputeShader> compute;
    Com<ID3D11VertexShader> vertex;
    Com<ID3D11PixelShader> pixel;
    Com<ID3D11Texture2D> image, hud, atlas;
    Com<ID3D11UnorderedAccessView> output;
    Com<ID3D11ShaderResourceView> imageView, hudView, triangleView, nodeView, atlasView;
    Com<ID3D11Buffer> constants, triangles, nodes;
    Com<ID3D11SamplerState> sampler;
    HDC dc = nullptr;
    HBITMAP bitmap = nullptr;
    HGDIOBJ oldBitmap = nullptr;
    void *bits = nullptr;
    bool software = false;
    std::wstring adapter;
    t_vec3 camera{0, 8, -12};

    ~Renderer();
    void init(HWND window);
    void level(const slide::Course &course);
    void ui(const slide::Game &game, bool muted, float fps);
    void draw_frame(const slide::Game &game, float delta);
    void screenshot(const fs::path &destination);
    static std::wstring wide(const std::string &utf8);
    static std::wstring time(float seconds);

  private:
    void draw_title();
    void draw_pause();
    void draw_defeat(const slide::Game &game);
    void draw_results(const slide::Game &game);

    void shader(const char *entry, const char *profile, Com<ID3DBlob> &code);
    void makeAtlas(const slide::Course &course);
    void rect(int x, int y, int width, int height, COLORREF color);
    void text(int x, int y, int size, const std::wstring &value,
              COLORREF color = RGB(226, 237, 239), bool bold = false);
    void wrapped(int x, int y, int width, int height, int size, const std::wstring &value,
                 COLORREF color, bool bold = false);
    template <class T>
    void buffer(const std::vector<T> &data, Com<ID3D11Buffer> &b,
                Com<ID3D11ShaderResourceView> &view)
    {
        D3D11_BUFFER_DESC d{};
        d.ByteWidth = UINT(data.size() * sizeof(T));
        d.Usage = D3D11_USAGE_IMMUTABLE;
        d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        d.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        d.StructureByteStride = sizeof(T);
        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = data.data();
        check(device->CreateBuffer(&d, &init, b.out()), "Upload mesh/BVH");
        D3D11_SHADER_RESOURCE_VIEW_DESC srv{};
        srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srv.Buffer.NumElements = (UINT)data.size();
        check(device->CreateShaderResourceView(b.p, &srv, view.out()), "Create mesh/BVH view");
    }
};
