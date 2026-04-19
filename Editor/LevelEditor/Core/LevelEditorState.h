#pragma once
#include "LevelEditorTypes.h"
#include "../Documents/MapDocument.h"
#include "../Selection/SelectionManager.h"
#include <string>
#include <vector>

namespace Figure::Editor::LevelEditor
{
    class LevelEditorState
    {
    public:
        EditorTool ActiveTool = EditorTool::Select;
        EditorView ActiveView = EditorView::Top;
        MapDocument Document;
        SelectionManager Selection;
        std::vector<std::wstring> Output;

        bool IsDrawingBlock = false;
        bool IsDraggingBlock = false;
        POINT DragStart{ 0, 0 };
        POINT LastMouse{ 0, 0 };
        RECT PreviewRect{ 0, 0, 0, 0 };

        void Log(const std::wstring& message);
        void ResetTransientInput();
    };
}
