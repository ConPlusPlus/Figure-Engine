#include "MapDocument.h"

namespace Figure { namespace Editor { namespace Anvil {

    void MapDocument::Clear()
    {
        m_blocks.clear();
    }

    int MapDocument::AddBlock(const RECT& bounds)
    {
        Block block{};
        block.Bounds = bounds;
        m_blocks.push_back(block);
        return static_cast<int>(m_blocks.size()) - 1;
    }

    void MapDocument::RemoveBlock(int index)
    {
        if (!IsValidIndex(index))
        {
            return;
        }

        m_blocks.erase(m_blocks.begin() + index);
    }

    bool MapDocument::IsValidIndex(int index) const
    {
        return index >= 0 && index < static_cast<int>(m_blocks.size());
    }

    std::vector<Block>& MapDocument::Blocks()
    {
        return m_blocks;
    }

    const std::vector<Block>& MapDocument::Blocks() const
    {
        return m_blocks;
    }

} } }
