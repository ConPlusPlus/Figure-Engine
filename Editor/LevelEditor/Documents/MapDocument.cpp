#include "MapDocument.h"
#include <fstream>

namespace Figure::Editor::LevelEditor
{
    void MapDocument::New()
    {
        m_path.clear();
        m_blocks.clear();
        m_dirty = false;
    }

    bool MapDocument::LoadFromPath(const std::wstring& path)
    {
        std::wifstream file(path);
        if (!file.is_open())
        {
            return false;
        }

        m_blocks.clear();

        std::wstring keyword;
        while (file >> keyword)
        {
            if (keyword == L"FIGMAP")
            {
                int version = 0;
                file >> version;
            }
            else if (keyword == L"BLOCK")
            {
                int x = 0, y = 0, w = 0, h = 0;
                file >> x >> y >> w >> h;
                MapBlock block{};
                block.Rect = { x, y, x + w, y + h };
                m_blocks.push_back(block);
            }
        }

        m_path = path;
        m_dirty = false;
        return true;
    }

    bool MapDocument::SaveToPath(const std::wstring& path) const
    {
        std::wofstream file(path);
        if (!file.is_open())
        {
            return false;
        }

        file << L"FIGMAP 1\n";
        for (const auto& block : m_blocks)
        {
            RECT r = block.Rect;
            int left = min(r.left, r.right);
            int top = min(r.top, r.bottom);
            int right = max(r.left, r.right);
            int bottom = max(r.top, r.bottom);
            file << L"BLOCK " << left << L" " << top << L" " << (right - left) << L" " << (bottom - top) << L"\n";
        }

        return true;
    }

    bool MapDocument::Save()
    {
        if (m_path.empty())
        {
            return false;
        }

        if (!SaveToPath(m_path))
        {
            return false;
        }

        m_dirty = false;
        return true;
    }

    bool MapDocument::SaveAs(const std::wstring& path)
    {
        if (!SaveToPath(path))
        {
            return false;
        }

        m_path = path;
        m_dirty = false;
        return true;
    }

    std::vector<MapBlock>& MapDocument::GetBlocks()
    {
        return m_blocks;
    }

    const std::vector<MapBlock>& MapDocument::GetBlocks() const
    {
        return m_blocks;
    }

    const std::wstring& MapDocument::GetPath() const
    {
        return m_path;
    }

    bool MapDocument::IsDirty() const
    {
        return m_dirty;
    }

    void MapDocument::SetDirty(bool dirty)
    {
        m_dirty = dirty;
    }
}
