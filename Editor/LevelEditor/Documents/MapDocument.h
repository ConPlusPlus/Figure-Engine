#pragma once
#include "../Core/LevelEditorTypes.h"
#include <string>
#include <vector>

namespace Figure::Editor::LevelEditor
{
    class MapDocument
    {
    public:
        void New();
        bool LoadFromPath(const std::wstring& path);
        bool SaveToPath(const std::wstring& path) const;
        bool Save();
        bool SaveAs(const std::wstring& path);

        std::vector<MapBlock>& GetBlocks();
        const std::vector<MapBlock>& GetBlocks() const;

        const std::wstring& GetPath() const;
        bool IsDirty() const;
        void SetDirty(bool dirty);

    private:
        std::wstring m_path;
        std::vector<MapBlock> m_blocks;
        bool m_dirty = false;
    };
}
