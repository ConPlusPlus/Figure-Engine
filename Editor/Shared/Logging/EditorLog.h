#pragma once
#include <string>
#include <vector>

namespace Figure::Editor::Shared
{
    class EditorLog
    {
    public:
        void Write(const std::wstring& message);
        void Clear();
        const std::vector<std::wstring>& GetMessages() const;

    private:
        std::vector<std::wstring> m_messages;
    };
}
