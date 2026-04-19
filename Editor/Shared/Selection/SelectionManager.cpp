#include "SelectionManager.h"
#include <algorithm>

namespace Figure::Editor::Shared
{
    void SelectionManager::Clear() { m_selectedObjectIds.clear(); }

    void SelectionManager::SetSingleSelection(int objectId)
    {
        m_selectedObjectIds.clear();
        m_selectedObjectIds.push_back(objectId);
    }

    void SelectionManager::AddSelection(int objectId)
    {
        if (!IsSelected(objectId)) { m_selectedObjectIds.push_back(objectId); }
    }

    void SelectionManager::RemoveSelection(int objectId)
    {
        m_selectedObjectIds.erase(
            std::remove(m_selectedObjectIds.begin(), m_selectedObjectIds.end(), objectId),
            m_selectedObjectIds.end());
    }

    bool SelectionManager::IsSelected(int objectId) const
    {
        return std::find(m_selectedObjectIds.begin(), m_selectedObjectIds.end(), objectId) != m_selectedObjectIds.end();
    }

    const std::vector<int>& SelectionManager::GetSelection() const
    {
        return m_selectedObjectIds;
    }
}
