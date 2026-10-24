#include "renderer.hpp"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace slide;
void Renderer::shader(const char *entry, const char *profile, Com<ID3DBlob> &code)
{
    Com<ID3DBlob> errors;
    HRESULT hr = D3DCompileFromFile((directory() / L"shaders" / L"draw_frame.hlsl").c_str(),
                                    nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, profile,
                                    D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3,
                                    0, code.out(), errors.out());
    if (FAILED(hr))
        throw std::runtime_error(
            errors.p ? std::string((char *)errors->GetBufferPointer(), errors->GetBufferSize())
                     : "Cannot load trace.hlsl beside Slide.exe");
}

void Renderer::init(HWND window)
{
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferDesc.Width = 1280;
    desc.BufferDesc.Height = 720;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.OutputWindow = window;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL requested = D3D_FEATURE_LEVEL_11_0, obtained{};
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                               &requested, 1, D3D11_SDK_VERSION, &desc, swap.out(),
                                               device.out(), &obtained, context.out());
    if (FAILED(hr))
    {
        software = true;
        check(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, &requested,
                                            1, D3D11_SDK_VERSION, &desc, swap.out(), device.out(),
                                            &obtained, context.out()),
              "Create D3D11 device");
    }
    Com<IDXGIDevice> dx;
    Com<IDXGIAdapter> ad;
    check(device->QueryInterface(__uuidof(IDXGIDevice), (void **)dx.out()), "Query DXGI");
    check(dx->GetAdapter(ad.out()), "Get adapter");
    DXGI_ADAPTER_DESC a{};
    ad->GetDesc(&a);
    adapter = a.Description;
    Com<ID3D11Texture2D> back;
    check(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)back.out()), "Get backbuffer");
    check(device->CreateRenderTargetView(back.p, nullptr, target.out()), "Create target");
    D3D11_TEXTURE2D_DESC td{};
    td.Width = W;
    td.Height = H;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    check(device->CreateTexture2D(&td, nullptr, image.out()), "Create ray image");
    check(device->CreateUnorderedAccessView(image.p, nullptr, output.out()), "Create output");
    check(device->CreateShaderResourceView(image.p, nullptr, imageView.out()), "Create image view");
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    check(device->CreateTexture2D(&td, nullptr, hud.out()), "Create HUD texture");
    check(device->CreateShaderResourceView(hud.p, nullptr, hudView.out()), "Create HUD view");
    td.Width = 512;
    td.Height = 1024;
    check(device->CreateTexture2D(&td, nullptr, atlas.out()), "Create message atlas");
    check(device->CreateShaderResourceView(atlas.p, nullptr, atlasView.out()),
          "Create message atlas view");
    D3D11_BUFFER_DESC cb{};
    cb.ByteWidth = sizeof(Frame);
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    check(device->CreateBuffer(&cb, nullptr, constants.out()), "Create constants");
    Com<ID3DBlob> cs, vs, ps;
    shader("draw_frame", "cs_5_0", cs);
    shader("VSMain", "vs_5_0", vs);
    shader("PSMain", "ps_5_0", ps);
    check(device->CreateComputeShader(cs->GetBufferPointer(), cs->GetBufferSize(), nullptr,
                                      compute.out()),
          "Create compute shader");
    check(device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr,
                                     vertex.out()),
          "Create vertex shader");
    check(device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr,
                                    pixel.out()),
          "Create pixel shader");
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    check(device->CreateSamplerState(&sd, sampler.out()), "Create sampler");
    dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = W;
    bi.bmiHeader.biHeight = -H;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!dc || !bitmap)
        throw std::runtime_error("Could not create HUD surface");
    oldBitmap = SelectObject(dc, bitmap);
    SetBkMode(dc, TRANSPARENT);
}

void Renderer::level(const Course &c)
{
    Mesh mesh(c);
    buffer(mesh.triangles, triangles, triangleView);
    buffer(mesh.nodes, nodes, nodeView);
    camera = c.spawn + t_vec3{0, 8, -12};
    makeAtlas(c);
}
Renderer::~Renderer()
{
    if (dc)
    {
        SelectObject(dc, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(dc);
    }
}
