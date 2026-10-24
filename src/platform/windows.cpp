#include "windows.hpp"
#include <sstream>
void check(HRESULT result, const char *operation)
{
    if (SUCCEEDED(result))
        return;
    std::ostringstream message;
    message << operation << " (0x" << std::hex << static_cast<unsigned long>(result) << ")";
    throw std::runtime_error(message.str());
}
fs::path directory()
{
    wchar_t path[32768];
    GetModuleFileNameW(nullptr, path, 32768);
    return fs::path(path).parent_path();
}
