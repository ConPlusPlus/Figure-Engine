#include "SelectionManager.h"

namespace Figure { namespace Editor { namespace Anvil {

    void SelectionManager::Clear()
    {
        m_selectedIndex = -1;
    }

    void SelectionManager::SetSelectedIndex(int index)
    {
        m_selectedIndex = index;
    }

    int SelectionManager::GetSelectedIndex() const
    {
        return m_selectedIndex;
    }

    bool SelectionManager::HasSelection() const
    {
        return m_selectedIndex >= 0;
    }

} } }
