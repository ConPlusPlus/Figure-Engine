#include "LevelEditorState.h"

namespace Figure::Editor::LevelEditor
{
    void LevelEditorState::Log(const std::wstring& message)
    {
        Output.push_back(message);
        if (Output.size() > 120)
        {
            Output.erase(Output.begin(), Output.begin() + 20);
        }
    }

    void LevelEditorState::ResetTransientInput()
    {
        IsDrawingBlock = false;
        IsDraggingBlock = false;
        PreviewRect = { 0, 0, 0, 0 };
    }
}
