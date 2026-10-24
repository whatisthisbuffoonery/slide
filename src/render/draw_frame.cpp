#include "renderer.hpp"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace slide;
void Renderer::draw_frame(const Game &g, float delta)
{
    t_vec3 desired = g.position + t_vec3{0, 8, -12};
    camera += (desired - camera) * (1 - std::exp(-delta * 5));
    t_vec3 f = normalized(g.position + t_vec3{0, 0, 2} - camera),
           r = normalized(vec_cross({0, 1, 0}, f)), u = vec_cross(f, r);
    Frame frame{};
    frame.eye = F4(camera);
    frame.forward = F4(f);
    frame.right = F4(r);
    frame.up = F4(u);
    frame.ball = F4(g.position, .38f);
    frame.info = F4({W, H, g.worldTime}, (float)g.level);
    for (int i = 0; i < 4; ++i)
        frame.markers[i].w = -1;
    for (size_t i = 0; i < g.world.checkpoints.size(); ++i)
        frame.markers[i] = F4(g.world.checkpoints[i], i < g.collected    ? 2.f
                                                      : i == g.collected ? 1.f
                                                                         : 0.f);
    frame.markers[g.world.checkpoints.size()] =
        F4(g.world.goal, g.collected == g.world.checkpoints.size() ? 4.f : 3.f);
    for (int i = 0; i < (int)g.world.hazards.size(); ++i)
        frame.hazards[i] = F4(g.world.hazards[i].at(g.worldTime), 1.f);
    context->UpdateSubresource(constants.p, 0, nullptr, &frame, 0, 0);
    ID3D11ShaderResourceView *empty = nullptr;
    context->PSSetShaderResources(0, 1, &empty);
    context->CSSetShader(compute.p, nullptr, 0);
    context->CSSetConstantBuffers(0, 1, &constants.p);
    ID3D11ShaderResourceView *resources[] = {triangleView.p, nodeView.p, hudView.p, atlasView.p};
    context->CSSetShaderResources(0, 4, resources);
    context->CSSetUnorderedAccessViews(0, 1, &output.p, nullptr);
    context->Dispatch((W + 7) / 8, (H + 7) / 8, 1);
    ID3D11UnorderedAccessView *none = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &none, nullptr);
    ID3D11ShaderResourceView *nulls[4] = {};
    context->CSSetShaderResources(0, 4, nulls);
    context->OMSetRenderTargets(1, &target.p, nullptr);
    D3D11_VIEWPORT vp{0, 0, 1280, 720, 0, 1};
    context->RSSetViewports(1, &vp);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(vertex.p, nullptr, 0);
    context->PSSetShader(pixel.p, nullptr, 0);
    context->PSSetShaderResources(0, 1, &imageView.p);
    context->PSSetSamplers(0, 1, &sampler.p);
    context->Draw(3, 0);
    check(swap->Present(1, 0), "Present");
}

void Renderer::screenshot(const fs::path &path)
{
    D3D11_TEXTURE2D_DESC d{};
    image->GetDesc(&d);
    d.Usage = D3D11_USAGE_STAGING;
    d.BindFlags = 0;
    d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Com<ID3D11Texture2D> staging;
    check(device->CreateTexture2D(&d, nullptr, staging.out()), "Screenshot texture");
    context->CopyResource(staging.p, image.p);
    D3D11_MAPPED_SUBRESOURCE map{};
    check(context->Map(staging.p, 0, D3D11_MAP_READ, 0, &map), "Screenshot readback");
    std::vector<unsigned char> data(W * H * 4);
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x)
        {
            auto *s = (unsigned char *)map.pData + y * map.RowPitch + x * 4;
            auto *t = data.data() + (y * W + x) * 4;
            t[0] = s[2];
            t[1] = s[1];
            t[2] = s[0];
            t[3] = 255;
        }
    context->Unmap(staging.p, 0);
    BITMAPFILEHEADER file{};
    file.bfType = 0x4d42;
    file.bfOffBits = sizeof(file) + sizeof(BITMAPINFOHEADER);
    file.bfSize = file.bfOffBits + (DWORD)data.size();
    BITMAPINFOHEADER header{};
    header.biSize = sizeof(header);
    header.biWidth = W;
    header.biHeight = -H;
    header.biPlanes = 1;
    header.biBitCount = 32;
    std::ofstream out(path, std::ios::binary);
    out.write((char *)&file, sizeof(file));
    out.write((char *)&header, sizeof(header));
    out.write((char *)data.data(), data.size());
}