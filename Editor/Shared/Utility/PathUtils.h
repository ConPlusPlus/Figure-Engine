#pragma once
#include <string>

namespace Figure::Editor::Shared
{
    inline std::wstring GetFileNameFromPath(const std::wstring& path)
    {
        const size_t slash = path.find_last_of(L"\\/");
        return slash == std::wstring::npos ? path : path.substr(slash + 1);
    }

    inline std::wstring GetDirectoryFromPath(const std::wstring& path)
    {
        const size_t slash = path.find_last_of(L"\\/");
        return slash == std::wstring::npos ? L"" : path.substr(0, slash);
    }
}
