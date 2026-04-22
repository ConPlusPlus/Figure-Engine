#pragma once

#include "../Documents/MapDocument.h"
#include "../Selection/SelectionManager.h"
#include <windows.h>

namespace Figure { namespace Editor { namespace Anvil {

    class EditorRenderer
    {
    public:
        static void Draw(HDC hdc, const RECT& viewport, const MapDocument& document, const SelectionManager& selection, bool topView, const RECT* previewRect);
    };

} } }
