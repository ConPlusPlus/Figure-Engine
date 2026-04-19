#include "ViewportHelpers.h"
#include <algorithm>

namespace Figure::Editor::LevelEditor
{
    static constexpr int ToolbarHeight = 54;
    static constexpr int LeftPanelWidth = 180;
    static constexpr int RightPanelWidth = 280;
    static constexpr int BottomPanelHeight = 150;

    EditorLayout CalculateLayout(const RECT& clientRect)
    {
        EditorLayout layout{};
        layout.Toolbar = { 0, 0, clientRect.right, ToolbarHeight };
        layout.ToolPalette = { 0, ToolbarHeight, LeftPanelWidth, clientRect.bottom - BottomPanelHeight };
        layout.Viewport = { LeftPanelWidth, ToolbarHeight, clientRect.right - RightPanelWidth, clientRect.bottom - BottomPanelHeight };
        layout.Properties = { clientRect.right - RightPanelWidth, ToolbarHeight, clientRect.right, clientRect.bottom - BottomPanelHeight };
        layout.Output = { 0, clientRect.bottom - BottomPanelHeight, clientRect.right, clientRect.bottom };
        return layout;
    }

    ToolbarButtons CalculateToolbarButtons()
    {
        ToolbarButtons buttons{};
        int x = 12;
        buttons.NewMap = { x, 10, x + 76, 40 }; x += 84;
        buttons.OpenMap = { x, 10, x + 76, 40 }; x += 84;
        buttons.SaveMap = { x, 10, x + 76, 40 }; x += 96;
        buttons.SelectTool = { x, 10, x + 78, 40 }; x += 86;
        buttons.BlockTool = { x, 10, x + 78, 40 }; x += 96;
        buttons.ViewTop = { x, 10, x + 70, 40 }; x += 78;
        buttons.ViewPerspective = { x, 10, x + 102, 40 };
        return buttons;
    }

    RECT NormalizeRectValues(RECT rc)
    {
        if (rc.left > rc.right) std::swap(rc.left, rc.right);
        if (rc.top > rc.bottom) std::swap(rc.top, rc.bottom);
        return rc;
    }

    bool IsSmallRect(const RECT& rc)
    {
        return (rc.right - rc.left) < 6 || (rc.bottom - rc.top) < 6;
    }

    int HitTestBlock(const POINT& point, const std::vector<MapBlock>& blocks)
    {
        for (int i = static_cast<int>(blocks.size()) - 1; i >= 0; --i)
        {
            if (PtInRect(&blocks[i].Rect, point))
            {
                return i;
            }
        }
        return -1;
    }
}
