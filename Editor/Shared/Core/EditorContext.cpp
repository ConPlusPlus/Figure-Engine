#include "EditorContext.h"

namespace Figure::Editor::Shared
{
    void EditorContext::SetToolName(const std::wstring& value) { m_toolName = value; }
    const std::wstring& EditorContext::GetToolName() const { return m_toolName; }

    void EditorContext::SetProjectPath(const std::wstring& value) { m_projectPath = value; }
    const std::wstring& EditorContext::GetProjectPath() const { return m_projectPath; }
}
