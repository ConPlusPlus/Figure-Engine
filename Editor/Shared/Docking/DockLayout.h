#pragma once
#include "DockNode.h"

namespace Figure::Editor::Shared
{
    class DockLayout
    {
    public:
        void Reset();
        void SetRoot(const std::shared_ptr<DockNode>& root);
        std::shared_ptr<DockNode> GetRoot() const;

        std::shared_ptr<DockNode> CreateLeaf(const std::wstring& panelId) const;
        std::shared_ptr<DockNode> CreateSplit(
            const std::shared_ptr<DockNode>& first,
            const std::shared_ptr<DockNode>& second,
            DockDirection direction,
            float splitRatio) const;

    private:
        std::shared_ptr<DockNode> m_root;
    };
}
