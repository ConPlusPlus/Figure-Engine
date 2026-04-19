#pragma once
#include "../Core/LevelEditorState.h"
#include "../Viewports/ViewportHelpers.h"
#include <windows.h>

namespace Figure::Editor::LevelEditor
{
    class EditorRenderer
    {
    public:
        static void Draw(HWND hWnd, HDC hdc, const LevelEditorState& state);
    };
}
