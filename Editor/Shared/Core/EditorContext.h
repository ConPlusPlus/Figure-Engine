#pragma once
#include <string>

namespace Figure::Editor::Shared
{
    class PanelManager;
    class CommandHistory;
    class SelectionManager;
    class PropertyInspector;
    class EditorLog;

    class EditorContext
    {
    public:
        void SetToolName(const std::wstring& value);
        const std::wstring& GetToolName() const;

        void SetProjectPath(const std::wstring& value);
        const std::wstring& GetProjectPath() const;

        PanelManager* Panels = nullptr;
        CommandHistory* Commands = nullptr;
        SelectionManager* Selection = nullptr;
        PropertyInspector* Inspector = nullptr;
        EditorLog* Log = nullptr;

    private:
        std::wstring m_toolName;
        std::wstring m_projectPath;
    };
}
