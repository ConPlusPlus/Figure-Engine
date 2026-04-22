#include "FigMapSerializer.h"
#include <fstream>

namespace Figure { namespace Editor { namespace Anvil {

    bool FigMapSerializer::Save(const std::wstring& path, const MapDocument& document)
    {
        std::wofstream file(path);
        if (!file.is_open())
        {
            return false;
        }

        file << L"FIGMAP 1\n";
        for (const auto& block : document.Blocks())
        {
            const RECT& r = block.Bounds;
            file << L"BLOCK " << r.left << L" " << r.top << L" " << (r.right - r.left) << L" " << (r.bottom - r.top) << L"\n";
        }

        return true;
    }

    bool FigMapSerializer::Load(const std::wstring& path, MapDocument& document)
    {
        std::wifstream file(path);
        if (!file.is_open())
        {
            return false;
        }

        document.Clear();
        std::wstring token;
        while (file >> token)
        {
            if (token == L"FIGMAP")
            {
                int version = 0;
                file >> version;
            }
            else if (token == L"BLOCK")
            {
                int x = 0, y = 0, w = 0, h = 0;
                file >> x >> y >> w >> h;
                RECT r{ x, y, x + w, y + h };
                document.AddBlock(r);
            }
        }

        return true;
    }

} } }
