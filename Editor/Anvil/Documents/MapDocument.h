#pragma once

#include "../Core/AnvilTypes.h"

namespace Figure { namespace Editor { namespace Anvil {

    class MapDocument
    {
    public:
        void Clear();
        int AddBlock(const RECT& bounds);
        void RemoveBlock(int index);
        bool IsValidIndex(int index) const;

        std::vector<Block>& Blocks();
        const std::vector<Block>& Blocks() const;

    private:
        std::vector<Block> m_blocks;
    };

} } }
