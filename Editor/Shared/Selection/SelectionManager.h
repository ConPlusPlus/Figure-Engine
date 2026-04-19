#pragma once
#include <vector>

namespace Figure::Editor::Shared
{
    class SelectionManager
    {
    public:
        void Clear();
        void SetSingleSelection(int objectId);
        void AddSelection(int objectId);
        void RemoveSelection(int objectId);
        bool IsSelected(int objectId) const;
        const std::vector<int>& GetSelection() const;

    private:
        std::vector<int> m_selectedObjectIds;
    };
}
