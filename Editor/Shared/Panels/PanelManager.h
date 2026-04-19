#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Figure::Editor::Shared
{
    class EditorPanel;

    class PanelManager
    {
    public:
        void RegisterPanel(const std::shared_ptr<EditorPanel>& panel);
        std::shared_ptr<EditorPanel> FindPanel(const std::wstring& panelId) const;
        void SetPanelVisible(const std::wstring& panelId, bool visible);
        const std::vector<std::shared_ptr<EditorPanel>>& GetPanels() const;

    private:
        std::vector<std::shared_ptr<EditorPanel>> m_panels;
    };
}
