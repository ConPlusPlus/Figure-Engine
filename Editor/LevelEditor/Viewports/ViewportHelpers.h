#pragma once
#include "../Core/LevelEditorTypes.h"
#include <windows.h>

namespace Figure::Editor::LevelEditor
{
    EditorLayout CalculateLayout(const RECT& clientRect);
    ToolbarButtons CalculateToolbarButtons();
    RECT NormalizeRectValues(RECT rc);
    bool IsSmallRect(const RECT& rc);
    int HitTestBlock(const POINT& point, const std::vector<MapBlock>& blocks);
}
