#include "FigMapSerializer.h"

namespace Figure::Editor::LevelEditor
{
    bool FigMapSerializer::Load(const std::wstring& path, MapDocument& document)
    {
        return document.LoadFromPath(path);
    }

    bool FigMapSerializer::Save(const std::wstring& path, const MapDocument& document)
    {
        return document.SaveToPath(path);
    }
}
