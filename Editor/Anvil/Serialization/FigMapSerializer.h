#pragma once

#include "../Documents/MapDocument.h"
#include <string>

namespace Figure { namespace Editor { namespace Anvil {

    class FigMapSerializer
    {
    public:
        static bool Save(const std::wstring& path, const MapDocument& document);
        static bool Load(const std::wstring& path, MapDocument& document);
    };

} } }
