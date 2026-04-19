#include "EditorLog.h"

namespace Figure::Editor::Shared
{
    void EditorLog::Write(const std::wstring& message) { m_messages.push_back(message); }
    void EditorLog::Clear() { m_messages.clear(); }
    const std::vector<std::wstring>& EditorLog::GetMessages() const { return m_messages; }
}
