#pragma once
#include "../Documents/MapDocument.h"
#include <string>

namespace Figure::Editor::LevelEditor
{
    class FigMapSerializer
    {
    public:
        static bool Load(const std::wstring& path, MapDocument& document);
        static bool Save(const std::wstring& path, const MapDocument& document);
    };
}
