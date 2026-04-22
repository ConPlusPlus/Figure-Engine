#include "AnvilWindow.h"
#include "../framework.h"
#include "../resource.h"
#include "../Serialization/FigMapSerializer.h"
#include "../Rendering/EditorRenderer.h"

namespace Figure { namespace Editor { namespace Anvil {

    static const wchar_t* kClassName = L"FigureAnvilWindow";

    AnvilWindow::AnvilWindow()
    {
        Log(L"Anvil started.");
    }

    bool AnvilWindow::Create(HINSTANCE instance, int nCmdShow)
    {
        m_instance = instance;

        WNDCLASSEXW wcex{};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = AnvilWindow::WndProc;
        wcex.hInstance = instance;
        wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ANVIL);
        wcex.lpszClassName = kClassName;
        wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        RegisterClassExW(&wcex);

        m_hWnd = CreateWindowW(kClassName, L"Anvil", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 1400, 900, nullptr, nullptr, instance, this);
        if (!m_hWnd)
        {
            return false;
        }

        ShowWindow(m_hWnd, nCmdShow);
        UpdateWindow(m_hWnd);
        return true;
    }

    HWND AnvilWindow::Handle() const
    {
        return m_hWnd;
    }

    LRESULT CALLBACK AnvilWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        AnvilWindow* self = nullptr;
        if (message == WM_NCCREATE)
        {
            auto createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = reinterpret_cast<AnvilWindow*>(createStruct->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->m_hWnd = hWnd;
        }
        else
        {
            self = reinterpret_cast<AnvilWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (!self)
        {
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }

        switch (message)
        {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDM_ABOUT:
                MessageBoxW(hWnd, L"Anvil - Figure Engine level editor", L"About Anvil", MB_OK | MB_ICONINFORMATION);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_FILE_NEW_MAP:
                self->OnNewMap();
                break;
            case IDM_FILE_OPEN_MAP:
                self->OnOpenMap();
                break;
            case IDM_FILE_SAVE_MAP:
                self->OnSaveMap(false);
                break;
            case IDM_FILE_SAVE_MAP_AS:
                self->OnSaveMap(true);
                break;
            case IDM_TOOL_SELECT:
                self->m_tool = ToolKind::Select;
                self->Log(L"Tool changed to Select.");
                InvalidateRect(hWnd, nullptr, TRUE);
                break;
            case IDM_TOOL_BLOCK:
                self->m_tool = ToolKind::Block;
                self->Log(L"Tool changed to Block.");
                InvalidateRect(hWnd, nullptr, TRUE);
                break;
            case IDM_TOOL_ENTITY:
                MessageBoxW(hWnd, L"Entity tool is planned for Phase 2.", L"Anvil", MB_OK | MB_ICONINFORMATION);
                break;
            case IDM_TOOL_TEXTURE:
                MessageBoxW(hWnd, L"Texture tool is planned for Phase 2.", L"Anvil", MB_OK | MB_ICONINFORMATION);
                break;
            case IDM_VIEW_TOP:
                self->m_view = ViewKind::Top;
                InvalidateRect(hWnd, nullptr, TRUE);
                break;
            case IDM_VIEW_PERSPECTIVE:
                self->m_view = ViewKind::Perspective;
                InvalidateRect(hWnd, nullptr, TRUE);
                break;
            case IDM_EDIT_DELETE:
                self->OnDeleteSelection();
                break;
            default:
                return DefWindowProcW(hWnd, message, wParam, lParam);
            }
            return 0;
        case WM_LBUTTONDOWN:
            self->OnLeftButtonDown(POINT{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            return 0;
        case WM_LBUTTONUP:
            self->OnLeftButtonUp(POINT{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            return 0;
        case WM_MOUSEMOVE:
            self->OnMouseMove(POINT{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            return 0;
        case WM_PAINT:
            self->OnPaint();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    RECT AnvilWindow::ViewportRect() const
    {
        RECT client{};
        GetClientRect(m_hWnd, &client);
        return RECT{ 180, 54, client.right - 280, client.bottom - 150 };
    }

    RECT AnvilWindow::NormalizeRect(RECT rc) const
    {
        if (rc.left > rc.right) std::swap(rc.left, rc.right);
        if (rc.top > rc.bottom) std::swap(rc.top, rc.bottom);
        return rc;
    }

    int AnvilWindow::HitTestBlock(POINT pt) const
    {
        for (int i = static_cast<int>(m_document.Blocks().size()) - 1; i >= 0; --i)
        {
            if (PtInRect(&m_document.Blocks()[i].Bounds, pt))
            {
                return i;
            }
        }
        return -1;
    }

    void AnvilWindow::OnPaint()
    {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(m_hWnd, &ps);

        RECT client{};
        GetClientRect(m_hWnd, &client);
        RECT toolbar{ 0, 0, client.right, 54 };
        RECT tools{ 0, 54, 180, client.bottom - 150 };
        RECT props{ client.right - 280, 54, client.right, client.bottom - 150 };
        RECT output{ 0, client.bottom - 150, client.right, client.bottom };
        RECT viewport = ViewportRect();

        HBRUSH brush = CreateSolidBrush(RGB(245,245,245)); FillRect(hdc, &toolbar, brush); DeleteObject(brush);
        brush = CreateSolidBrush(RGB(248,248,248)); FillRect(hdc, &tools, brush); FillRect(hdc, &props, brush); DeleteObject(brush);
        brush = CreateSolidBrush(RGB(242,242,242)); FillRect(hdc, &output, brush); DeleteObject(brush);

        RECT preview = NormalizeRect(m_previewRect);
        const RECT* previewPtr = m_isDrawing ? &preview : nullptr;
        EditorRenderer::Draw(hdc, viewport, m_document, m_selection, m_view == ViewKind::Top, previewPtr);

        SetBkMode(hdc, TRANSPARENT);
        TextOutW(hdc, 12, 18, L"Anvil", 5);
        TextOutW(hdc, 220, 18, m_tool == ToolKind::Select ? L"Tool: Select" : L"Tool: Block", m_tool == ToolKind::Select ? 12 : 11);
        TextOutW(hdc, 12, 70, L"Tools", 5);
        TextOutW(hdc, 12, 95, L"Select: click block to move", 27);
        TextOutW(hdc, 12, 118, L"Block: click-drag to create", 28);
        TextOutW(hdc, props.left + 12, 70, L"Properties", 10);

        std::wstring selected = m_selection.HasSelection() ? std::to_wstring(m_selection.GetSelectedIndex()) : L"None";
        std::wstring selText = L"Selected: " + selected;
        TextOutW(hdc, props.left + 12, 95, selText.c_str(), static_cast<int>(selText.size()));

        std::wstring countText = L"Blocks: " + std::to_wstring(m_document.Blocks().size());
        TextOutW(hdc, props.left + 12, 118, countText.c_str(), static_cast<int>(countText.size()));

        std::wstring mapText = L"Map: " + (m_currentMapPath.empty() ? L"<unsaved>" : m_currentMapPath);
        TextOutW(hdc, props.left + 12, 141, mapText.c_str(), static_cast<int>(mapText.size()));

        TextOutW(hdc, 12, output.top + 12, L"Output", 6);
        int y = output.top + 36;
        size_t start = m_log.size() > 5 ? m_log.size() - 5 : 0;
        for (size_t i = start; i < m_log.size(); ++i)
        {
            TextOutW(hdc, 12, y, m_log[i].c_str(), static_cast<int>(m_log[i].size()));
            y += 20;
        }

        EndPaint(m_hWnd, &ps);
    }

    void AnvilWindow::OnLeftButtonDown(POINT pt)
    {
        RECT viewport = ViewportRect();
        if (!PtInRect(&viewport, pt)) return;
        if (m_tool == ToolKind::Block)
        {
            m_isDrawing = true;
            m_dragStart = pt;
            m_previewRect = RECT{ pt.x, pt.y, pt.x, pt.y };
            SetCapture(m_hWnd);
        }
        else
        {
            int hit = HitTestBlock(pt);
            m_selection.SetSelectedIndex(hit);
            if (hit >= 0)
            {
                m_isDragging = true;
                m_lastMouse = pt;
                SetCapture(m_hWnd);
            }
            InvalidateRect(m_hWnd, nullptr, TRUE);
        }
    }

    void AnvilWindow::OnLeftButtonUp(POINT)
    {
        if (m_isDrawing)
        {
            RECT normalized = NormalizeRect(m_previewRect);
            if ((normalized.right - normalized.left) >= 6 && (normalized.bottom - normalized.top) >= 6)
            {
                m_selection.SetSelectedIndex(m_document.AddBlock(normalized));
                Log(L"Block created.");
            }
            m_isDrawing = false;
            ReleaseCapture();
            InvalidateRect(m_hWnd, nullptr, TRUE);
        }
        else if (m_isDragging)
        {
            m_isDragging = false;
            ReleaseCapture();
            Log(L"Block moved.");
        }
    }

    void AnvilWindow::OnMouseMove(POINT pt)
    {
        if (m_isDrawing)
        {
            m_previewRect = RECT{ m_dragStart.x, m_dragStart.y, pt.x, pt.y };
            InvalidateRect(m_hWnd, nullptr, TRUE);
        }
        else if (m_isDragging && m_selection.HasSelection())
        {
            int index = m_selection.GetSelectedIndex();
            if (m_document.IsValidIndex(index))
            {
                int dx = pt.x - m_lastMouse.x;
                int dy = pt.y - m_lastMouse.y;
                OffsetRect(&m_document.Blocks()[index].Bounds, dx, dy);
                m_lastMouse = pt;
                InvalidateRect(m_hWnd, nullptr, TRUE);
            }
        }
    }

    void AnvilWindow::OnDeleteSelection()
    {
        if (!m_selection.HasSelection()) return;
        int index = m_selection.GetSelectedIndex();
        m_document.RemoveBlock(index);
        m_selection.Clear();
        Log(L"Deleted selection.");
        InvalidateRect(m_hWnd, nullptr, TRUE);
    }

    void AnvilWindow::OnNewMap()
    {
        m_document.Clear();
        m_selection.Clear();
        m_currentMapPath.clear();
        Log(L"New map created.");
        InvalidateRect(m_hWnd, nullptr, TRUE);
    }

    void AnvilWindow::OnOpenMap()
    {
        wchar_t path[MAX_PATH]{};
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = L"Figure Map (*.figmap)\0*.figmap\0All Files (*.*)\0*.*\0";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetOpenFileNameW(&ofn) && FigMapSerializer::Load(path, m_document))
        {
            m_currentMapPath = path;
            m_selection.Clear();
            Log(L"Map loaded.");
            InvalidateRect(m_hWnd, nullptr, TRUE);
        }
    }

    void AnvilWindow::OnSaveMap(bool saveAs)
    {
        if (saveAs || m_currentMapPath.empty())
        {
            wchar_t path[MAX_PATH]{};
            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = m_hWnd;
            ofn.lpstrFile = path;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Figure Map (*.figmap)\0*.figmap\0All Files (*.*)\0*.*\0";
            ofn.lpstrDefExt = L"figmap";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
            if (!GetSaveFileNameW(&ofn)) return;
            m_currentMapPath = path;
        }

        if (FigMapSerializer::Save(m_currentMapPath, m_document))
        {
            Log(L"Map saved.");
        }
    }

    void AnvilWindow::Log(const std::wstring& text)
    {
        m_log.push_back(text);
        if (m_log.size() > 64)
        {
            m_log.erase(m_log.begin(), m_log.begin() + 16);
        }
    }

} } }
