#pragma once

namespace Figure { namespace Editor { namespace Anvil {

    class SelectionManager
    {
    public:
        void Clear();
        void SetSelectedIndex(int index);
        int GetSelectedIndex() const;
        bool HasSelection() const;

    private:
        int m_selectedIndex = -1;
    };

} } }
