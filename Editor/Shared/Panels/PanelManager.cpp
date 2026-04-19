#include "PanelManager.h"
#include "EditorPanel.h"

namespace Figure::Editor::Shared
{
    void PanelManager::RegisterPanel(const std::shared_ptr<EditorPanel>& panel)
    {
        if (panel) { m_panels.push_back(panel); }
    }

    std::shared_ptr<EditorPanel> PanelManager::FindPanel(const std::wstring& panelId) const
    {
        for (const auto& panel : m_panels)
        {
            if (panel && panelId == panel->GetPanelId()) { return panel; }
        }
        return nullptr;
    }

    void PanelManager::SetPanelVisible(const std::wstring& panelId, bool visible)
    {
        auto panel = FindPanel(panelId);
        if (panel) { panel->SetVisible(visible); }
    }

    const std::vector<std::shared_ptr<EditorPanel>>& PanelManager::GetPanels() const
    {
        return m_panels;
    }
}
