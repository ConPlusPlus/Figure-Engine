#pragma once

#include "../Documents/MapDocument.h"
#include "../Selection/SelectionManager.h"
#include "../Core/AnvilTypes.h"
#include <string>
#include <vector>

namespace Figure { namespace Editor { namespace Anvil {

    class AnvilWindow
    {
    public:
        AnvilWindow();
        bool Create(HINSTANCE instance, int nCmdShow);
        HWND Handle() const;
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        void OnPaint();
        void OnLeftButtonDown(POINT pt);
        void OnLeftButtonUp(POINT pt);
        void OnMouseMove(POINT pt);
        void OnDeleteSelection();
        void OnNewMap();
        void OnOpenMap();
        void OnSaveMap(bool saveAs);
        int HitTestBlock(POINT pt) const;
        RECT ViewportRect() const;
        RECT NormalizeRect(RECT rc) const;
        void Log(const std::wstring& text);

        HINSTANCE m_instance = nullptr;
        HWND m_hWnd = nullptr;
        MapDocument m_document;
        SelectionManager m_selection;
        ToolKind m_tool = ToolKind::Select;
        ViewKind m_view = ViewKind::Top;
        bool m_isDrawing = false;
        bool m_isDragging = false;
        POINT m_dragStart{};
        POINT m_lastMouse{};
        RECT m_previewRect{};
        std::wstring m_currentMapPath;
        std::vector<std::wstring> m_log;
    };

} } }
