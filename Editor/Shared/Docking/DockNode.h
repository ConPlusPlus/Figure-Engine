#pragma once
#include <memory>
#include <string>

namespace Figure::Editor::Shared
{
    enum class DockDirection
    {
        Left,
        Right,
        Top,
        Bottom,
        Center
    };

    enum class DockNodeType
    {
        Leaf,
        Split
    };

    struct DockNode
    {
        DockNodeType Type = DockNodeType::Leaf;
        DockDirection Direction = DockDirection::Center;
        float SplitRatio = 0.5f;
        std::wstring PanelId;
        std::shared_ptr<DockNode> First;
        std::shared_ptr<DockNode> Second;
    };
}
