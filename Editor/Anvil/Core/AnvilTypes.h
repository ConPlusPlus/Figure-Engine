#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace Figure { namespace Editor { namespace Anvil {

    enum class ToolKind
    {
        Select,
        Block,
        Entity,
        Texture
    };

    enum class ViewKind
    {
        Top,
        Perspective
    };

    struct Block
    {
        RECT Bounds{};
    };

} } }
