#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <filesystem>
#include <stdexcept>
#include <string>
namespace fs = std::filesystem;
template <class T> struct Com
{
    T *p = nullptr;
    ~Com()
    {
        if (p)
            p->Release();
    }
    T *operator->() const
    {
        return p;
    }
    T **out()
    {
        if (p)
        {
            p->Release();
            p = nullptr;
        }
        return &p;
    }
    Com() = default;
    Com(const Com &) = delete;
    Com &operator=(const Com &) = delete;
};

void check(HRESULT result, const char *operation);
fs::path directory();
