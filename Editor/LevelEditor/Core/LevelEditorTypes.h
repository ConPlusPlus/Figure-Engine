#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace Figure::Editor::LevelEditor
{
    enum class EditorTool
    {
        Select,
        Block,
        Entity,
        Texture
    };

    enum class EditorView
    {
        Top,
        Perspective
    };

    struct MapBlock
    {
        RECT Rect{};
    };

    struct EditorLayout
    {
        RECT Toolbar{};
        RECT ToolPalette{};
        RECT Viewport{};
        RECT Properties{};
        RECT Output{};
    };

    struct ToolbarButtons
    {
        RECT NewMap{};
        RECT OpenMap{};
        RECT SaveMap{};
        RECT SelectTool{};
        RECT BlockTool{};
        RECT ViewTop{};
        RECT ViewPerspective{};
    };
}
