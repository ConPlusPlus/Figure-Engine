#pragma once
#include <windows.h>

namespace Figure::Editor::Shared
{
    inline RECT NormalizeRect(RECT rc)
    {
        if (rc.left > rc.right) { const int t = rc.left; rc.left = rc.right; rc.right = t; }
        if (rc.top > rc.bottom) { const int t = rc.top; rc.top = rc.bottom; rc.bottom = t; }
        return rc;
    }
}
