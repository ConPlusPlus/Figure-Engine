#include "DockLayout.h"

namespace Figure::Editor::Shared
{
    void DockLayout::Reset() { m_root.reset(); }
    void DockLayout::SetRoot(const std::shared_ptr<DockNode>& root) { m_root = root; }
    std::shared_ptr<DockNode> DockLayout::GetRoot() const { return m_root; }

    std::shared_ptr<DockNode> DockLayout::CreateLeaf(const std::wstring& panelId) const
    {
        auto node = std::make_shared<DockNode>();
        node->Type = DockNodeType::Leaf;
        node->PanelId = panelId;
        return node;
    }

    std::shared_ptr<DockNode> DockLayout::CreateSplit(
        const std::shared_ptr<DockNode>& first,
        const std::shared_ptr<DockNode>& second,
        DockDirection direction,
        float splitRatio) const
    {
        auto node = std::make_shared<DockNode>();
        node->Type = DockNodeType::Split;
        node->Direction = direction;
        node->SplitRatio = splitRatio;
        node->First = first;
        node->Second = second;
        return node;
    }
}
