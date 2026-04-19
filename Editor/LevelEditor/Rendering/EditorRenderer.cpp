#include "EditorRenderer.h"
#include <string>

namespace Figure::Editor::LevelEditor
{
    namespace
    {
        void FillColor(HDC hdc, RECT rc, COLORREF color)
        {
            HBRUSH brush = CreateSolidBrush(color);
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);
        }

        void DrawTextLine(HDC hdc, int x, int y, const std::wstring& text)
        {
            TextOutW(hdc, x, y, text.c_str(), static_cast<int>(text.size()));
        }

        std::wstring ToolName(EditorTool tool)
        {
            switch (tool)
            {
            case EditorTool::Select: return L"Select";
            case EditorTool::Block: return L"Block";
            case EditorTool::Entity: return L"Entity";
            case EditorTool::Texture: return L"Texture";
            }
            return L"Unknown";
        }

        std::wstring ViewName(EditorView view)
        {
            switch (view)
            {
            case EditorView::Top: return L"Top";
            case EditorView::Perspective: return L"Perspective";
            }
            return L"Unknown";
        }

        void DrawButton(HDC hdc, RECT rc, const std::wstring& text, bool active)
        {
            FillColor(hdc, rc, active ? RGB(70,110,180) : RGB(235,235,235));
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, active ? RGB(255,255,255) : RGB(20,20,20));
            DrawTextW(hdc, text.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        void DrawGrid(HDC hdc, const RECT& viewport)
        {
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(230,230,230));
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
        }
    }

    void EditorRenderer::Draw(HWND hWnd, HDC hdc, const LevelEditorState& state)
    {
        RECT client{};
        GetClientRect(hWnd, &client);

        const auto layout = CalculateLayout(client);
        const auto buttons = CalculateToolbarButtons();

        FillColor(hdc, layout.Toolbar, RGB(245,245,245));
        FillColor(hdc, layout.ToolPalette, RGB(248,248,248));
        FillColor(hdc, layout.Viewport, RGB(255,255,255));
        FillColor(hdc, layout.Properties, RGB(248,248,248));
        FillColor(hdc, layout.Output, RGB(242,242,242));

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(20,20,20));

        DrawButton(hdc, buttons.NewMap, L"New", false);
        DrawButton(hdc, buttons.OpenMap, L"Open", false);
        DrawButton(hdc, buttons.SaveMap, L"Save", false);
        DrawButton(hdc, buttons.SelectTool, L"Select", state.ActiveTool == EditorTool::Select);
        DrawButton(hdc, buttons.BlockTool, L"Block", state.ActiveTool == EditorTool::Block);
        DrawButton(hdc, buttons.ViewTop, L"Top", state.ActiveView == EditorView::Top);
        DrawButton(hdc, buttons.ViewPerspective, L"Perspective", state.ActiveView == EditorView::Perspective);

        DrawTextLine(hdc, 14, layout.ToolPalette.top + 12, L"Tools");
        DrawTextLine(hdc, 14, layout.ToolPalette.top + 40, L"Block: draw new blocks");
        DrawTextLine(hdc, 14, layout.ToolPalette.top + 60, L"Select: move and delete");
        DrawTextLine(hdc, 14, layout.ToolPalette.top + 92, L"Entity/Texture: reserved");

        DrawGrid(hdc, layout.Viewport);
        DrawTextLine(hdc, layout.Viewport.left + 14, layout.Viewport.top + 12, L"Map View - " + ViewName(state.ActiveView));
        DrawTextLine(hdc, layout.Viewport.left + 14, layout.Viewport.top + 32, L"Blocks: " + std::to_wstring(state.Document.GetBlocks().size()));

        for (size_t i = 0; i < state.Document.GetBlocks().size(); ++i)
        {
            RECT r = NormalizeRectValues(state.Document.GetBlocks()[i].Rect);
            HBRUSH brush = CreateSolidBrush((int)i == state.Selection.GetSelectedIndex() ? RGB(120,170,230) : RGB(180,200,225));
            FillRect(hdc, &r, brush);
            DeleteObject(brush);
            FrameRect(hdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        if (state.IsDrawingBlock)
        {
            RECT preview = NormalizeRectValues(state.PreviewRect);
            HBRUSH brush = CreateSolidBrush(RGB(210,220,235));
            FillRect(hdc, &preview, brush);
            DeleteObject(brush);
            FrameRect(hdc, &preview, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 12, L"Properties");
        DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 40, L"Tool: " + ToolName(state.ActiveTool));
        DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 62, L"Map: " + (state.Document.GetPath().empty() ? std::wstring(L"<unsaved>") : state.Document.GetPath()));
        DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 84, L"Dirty: " + std::wstring(state.Document.IsDirty() ? L"Yes" : L"No"));

        const int selected = state.Selection.GetSelectedIndex();
        DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 106, L"Selected Block: " + (selected >= 0 ? std::to_wstring(selected) : std::wstring(L"None")));
        if (selected >= 0 && selected < (int)state.Document.GetBlocks().size())
        {
            RECT r = NormalizeRectValues(state.Document.GetBlocks()[selected].Rect);
            DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 128, L"X: " + std::to_wstring(r.left));
            DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 148, L"Y: " + std::to_wstring(r.top));
            DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 168, L"Width: " + std::to_wstring(r.right - r.left));
            DrawTextLine(hdc, layout.Properties.left + 14, layout.Properties.top + 188, L"Height: " + std::to_wstring(r.bottom - r.top));
        }

        DrawTextLine(hdc, 14, layout.Output.top + 12, L"Output");
        int y = layout.Output.top + 36;
        const auto& output = state.Output;
        size_t start = output.size() > 5 ? output.size() - 5 : 0;
        for (size_t i = start; i < output.size(); ++i)
        {
            DrawTextLine(hdc, 14, y, output[i]);
            y += 20;
        }
    }
}
