#pragma once

namespace Figure::Editor::LevelEditor
{
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
}
