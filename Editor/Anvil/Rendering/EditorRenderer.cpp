#include "EditorRenderer.h"

namespace Figure { namespace Editor { namespace Anvil {

    static void FillRectColor(HDC hdc, const RECT& rc, COLORREF color)
    {
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);
    }

    void EditorRenderer::Draw(HDC hdc, const RECT& viewport, const MapDocument& document, const SelectionManager& selection, bool topView, const RECT* previewRect)
    {
        FillRectColor(hdc, viewport, RGB(255, 255, 255));

        HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(232, 232, 232));
        HPEN oldPen = (HPEN)SelectObject(hdc, gridPen);

        for (int x = viewport.left; x < viewport.right; x += 32)
        {
            MoveToEx(hdc, x, viewport.top, nullptr);
            LineTo(hdc, x, viewport.bottom);
        }
        for (int y = viewport.top; y < viewport.bottom; y += 32)
        {
            MoveToEx(hdc, viewport.left, y, nullptr);
            LineTo(hdc, viewport.right, y);
        }

        SelectObject(hdc, oldPen);
        DeleteObject(gridPen);

        for (int i = 0; i < static_cast<int>(document.Blocks().size()); ++i)
        {
            RECT r = document.Blocks()[i].Bounds;
            HBRUSH brush = CreateSolidBrush(selection.GetSelectedIndex() == i ? RGB(125, 170, 230) : RGB(185, 205, 230));
            FillRect(hdc, &r, brush);
            DeleteObject(brush);
            FrameRect(hdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        if (previewRect)
        {
            HBRUSH brush = CreateSolidBrush(RGB(210, 220, 235));
            FillRect(hdc, previewRect, brush);
            DeleteObject(brush);
            FrameRect(hdc, previewRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        SetBkMode(hdc, TRANSPARENT);
        TextOutW(hdc, viewport.left + 10, viewport.top + 10, topView ? L"Top View" : L"Perspective View", topView ? 8 : 16);
    }

} } }
