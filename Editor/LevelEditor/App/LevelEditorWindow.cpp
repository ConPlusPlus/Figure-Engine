#include "LevelEditorWindow.h"
#include "../Rendering/EditorRenderer.h"
#include "../Viewports/ViewportHelpers.h"
#include <memory>

namespace Figure::Editor::LevelEditor
{
    namespace
    {
        constexpr wchar_t WindowClassName[] = L"FigureLevelEditorWindow";
        HINSTANCE g_instance = nullptr;
    }

    bool LevelEditorWindow::Register(HINSTANCE hInstance)
    {
        g_instance = hInstance;

        WNDCLASSEXW wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FIGURELEVELEDITOR);
        wcex.lpszClassName = WindowClassName;
        wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        return RegisterClassExW(&wcex) != 0;
    }

    HWND LevelEditorWindow::Create(HINSTANCE hInstance, int nCmdShow)
    {
        auto* state = new LevelEditorState();
        state->Log(L"Level Editor started.");
        state->Log(L"Use Block tool to draw blocks.");
        state->Log(L"Use Select tool to move or delete blocks.");

        HWND hWnd = CreateWindowW(
            WindowClassName,
            L"Figure Level Editor",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0,
            1500, 900,
            nullptr, nullptr,
            hInstance,
            state);

        if (hWnd)
        {
            ShowWindow(hWnd, nCmdShow);
            UpdateWindow(hWnd);
        }

        return hWnd;
    }

    LevelEditorState* LevelEditorWindow::GetState(HWND hWnd)
    {
        return reinterpret_cast<LevelEditorState*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    bool LevelEditorWindow::SaveCurrentMap(HWND hWnd, LevelEditorState& state, bool saveAs)
    {
        if (saveAs || state.Document.GetPath().empty())
        {
            wchar_t fileBuffer[MAX_PATH] = {};
            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = fileBuffer;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Figure Map (*.figmap)\0*.figmap\0All Files (*.*)\0*.*\0";
            ofn.lpstrTitle = saveAs ? L"Save Figure Map As" : L"Save Figure Map";
            ofn.lpstrDefExt = L"figmap";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (!GetSaveFileNameW(&ofn))
            {
                return false;
            }

            if (!state.Document.SaveAs(fileBuffer))
            {
                MessageBoxW(hWnd, L"Failed to save map file.", L"Figure Level Editor", MB_OK | MB_ICONERROR);
                return false;
            }
        }
        else
        {
            if (!state.Document.Save())
            {
                MessageBoxW(hWnd, L"Failed to save map file.", L"Figure Level Editor", MB_OK | MB_ICONERROR);
                return false;
            }
        }

        state.Log(L"Saved map.");
        InvalidateRect(hWnd, nullptr, TRUE);
        return true;
    }

    void LevelEditorWindow::DeleteSelection(HWND hWnd, LevelEditorState& state)
    {
        const int index = state.Selection.GetSelectedIndex();
        if (index >= 0 && index < (int)state.Document.GetBlocks().size())
        {
            auto& blocks = state.Document.GetBlocks();
            blocks.erase(blocks.begin() + index);
            state.Selection.Clear();
            state.Document.SetDirty(true);
            state.Log(L"Deleted block.");
            InvalidateRect(hWnd, nullptr, TRUE);
        }
    }

    void LevelEditorWindow::HandleCommand(HWND hWnd, LevelEditorState& state, int commandId)
    {
        switch (commandId)
        {
        case IDM_ABOUT:
            MessageBoxW(hWnd, L"Figure Level Editor\nStarter block-out editor.", L"Figure Level Editor", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_FILE_NEW_MAP:
            state.Document.New();
            state.Selection.Clear();
            state.ResetTransientInput();
            state.Log(L"New map created.");
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        case IDM_FILE_OPEN_MAP:
        {
            wchar_t fileBuffer[MAX_PATH] = {};
            OPENFILENAMEW ofn{};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = fileBuffer;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Figure Map (*.figmap)\0*.figmap\0All Files (*.*)\0*.*\0";
            ofn.lpstrTitle = L"Open Figure Map";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameW(&ofn))
            {
                if (state.Document.LoadFromPath(fileBuffer))
                {
                    state.Selection.Clear();
                    state.Log(L"Opened map.");
                    InvalidateRect(hWnd, nullptr, TRUE);
                }
                else
                {
                    MessageBoxW(hWnd, L"Failed to open map file.", L"Figure Level Editor", MB_OK | MB_ICONERROR);
                }
            }
        }
        break;
        case IDM_FILE_SAVE_MAP:
            SaveCurrentMap(hWnd, state, false);
            break;
        case IDM_FILE_SAVE_MAP_AS:
            SaveCurrentMap(hWnd, state, true);
            break;
        case IDM_TOOL_SELECT:
            state.ActiveTool = EditorTool::Select;
            state.Log(L"Switched to Select tool.");
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        case IDM_TOOL_BLOCK:
            state.ActiveTool = EditorTool::Block;
            state.Log(L"Switched to Block tool.");
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        case IDM_TOOL_ENTITY:
            state.ActiveTool = EditorTool::Entity;
            state.Log(L"Entity tool is reserved for future work.");
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        case IDM_TOOL_TEXTURE:
            state.ActiveTool = EditorTool::Texture;
            state.Log(L"Texture tool is reserved for future work.");
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        case IDM_VIEW_TOP:
            state.ActiveView = EditorView::Top;
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        case IDM_VIEW_PERSPECTIVE:
            state.ActiveView = EditorView::Perspective;
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        case IDM_EDIT_DELETE:
            DeleteSelection(hWnd, state);
            break;
        }
    }

    void LevelEditorWindow::HandleLeftButtonDown(HWND hWnd, LevelEditorState& state, POINT point)
    {
        const RECT client = [] (HWND wnd) { RECT rc{}; GetClientRect(wnd, &rc); return rc; }(hWnd);
        const auto layout = CalculateLayout(client);
        const auto buttons = CalculateToolbarButtons();

        if (PtInRect(&buttons.NewMap, point)) { HandleCommand(hWnd, state, IDM_FILE_NEW_MAP); return; }
        if (PtInRect(&buttons.OpenMap, point)) { HandleCommand(hWnd, state, IDM_FILE_OPEN_MAP); return; }
        if (PtInRect(&buttons.SaveMap, point)) { HandleCommand(hWnd, state, IDM_FILE_SAVE_MAP); return; }
        if (PtInRect(&buttons.SelectTool, point)) { HandleCommand(hWnd, state, IDM_TOOL_SELECT); return; }
        if (PtInRect(&buttons.BlockTool, point)) { HandleCommand(hWnd, state, IDM_TOOL_BLOCK); return; }
        if (PtInRect(&buttons.ViewTop, point)) { HandleCommand(hWnd, state, IDM_VIEW_TOP); return; }
        if (PtInRect(&buttons.ViewPerspective, point)) { HandleCommand(hWnd, state, IDM_VIEW_PERSPECTIVE); return; }

        if (!PtInRect(&layout.Viewport, point))
        {
            return;
        }

        if (state.ActiveTool == EditorTool::Block)
        {
            state.IsDrawingBlock = true;
            state.DragStart = point;
            state.PreviewRect = { point.x, point.y, point.x, point.y };
            SetCapture(hWnd);
            return;
        }

        if (state.ActiveTool == EditorTool::Select)
        {
            const int hitIndex = HitTestBlock(point, state.Document.GetBlocks());
            state.Selection.SetSelectedIndex(hitIndex);
            if (hitIndex >= 0)
            {
                state.IsDraggingBlock = true;
                state.LastMouse = point;
                SetCapture(hWnd);
            }
            InvalidateRect(hWnd, nullptr, TRUE);
        }
    }

    void LevelEditorWindow::HandleMouseMove(HWND hWnd, LevelEditorState& state, POINT point)
    {
        if (state.IsDrawingBlock)
        {
            state.PreviewRect = { state.DragStart.x, state.DragStart.y, point.x, point.y };
            InvalidateRect(hWnd, nullptr, TRUE);
        }
        else if (state.IsDraggingBlock && state.Selection.HasSelection())
        {
            const int index = state.Selection.GetSelectedIndex();
            if (index >= 0 && index < (int)state.Document.GetBlocks().size())
            {
                auto& block = state.Document.GetBlocks()[index];
                const int dx = point.x - state.LastMouse.x;
                const int dy = point.y - state.LastMouse.y;
                OffsetRect(&block.Rect, dx, dy);
                state.LastMouse = point;
                state.Document.SetDirty(true);
                InvalidateRect(hWnd, nullptr, TRUE);
            }
        }
    }

    void LevelEditorWindow::HandleLeftButtonUp(HWND hWnd, LevelEditorState& state, POINT)
    {
        if (state.IsDrawingBlock)
        {
            RECT normalized = NormalizeRectValues(state.PreviewRect);
            if (!IsSmallRect(normalized))
            {
                MapBlock block{};
                block.Rect = normalized;
                state.Document.GetBlocks().push_back(block);
                state.Selection.SetSelectedIndex((int)state.Document.GetBlocks().size() - 1);
                state.Document.SetDirty(true);
                state.Log(L"Placed block.");
            }

            state.IsDrawingBlock = false;
            ReleaseCapture();
            InvalidateRect(hWnd, nullptr, TRUE);
        }
        else if (state.IsDraggingBlock)
        {
            state.IsDraggingBlock = false;
            ReleaseCapture();
            state.Log(L"Moved block.");
        }
    }

    void LevelEditorWindow::HandleRightButtonDown(HWND hWnd, LevelEditorState& state, POINT point)
    {
        const RECT client = [] (HWND wnd) { RECT rc{}; GetClientRect(wnd, &rc); return rc; }(hWnd);
        const auto layout = CalculateLayout(client);

        if (!PtInRect(&layout.Viewport, point))
        {
            return;
        }

        const int hitIndex = HitTestBlock(point, state.Document.GetBlocks());
        if (hitIndex >= 0)
        {
            auto& blocks = state.Document.GetBlocks();
            blocks.erase(blocks.begin() + hitIndex);
            state.Selection.Clear();
            state.Document.SetDirty(true);
            state.Log(L"Deleted block.");
            InvalidateRect(hWnd, nullptr, TRUE);
        }
    }

    LRESULT CALLBACK LevelEditorWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_NCCREATE:
        {
            auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            auto* state = reinterpret_cast<LevelEditorState*>(createStruct->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        }
        return TRUE;

        case WM_COMMAND:
        {
            auto* state = GetState(hWnd);
            if (state)
            {
                HandleCommand(hWnd, *state, LOWORD(wParam));
            }
        }
        return 0;

        case WM_LBUTTONDOWN:
        {
            auto* state = GetState(hWnd);
            if (state)
            {
                POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                HandleLeftButtonDown(hWnd, *state, pt);
            }
        }
        return 0;

        case WM_MOUSEMOVE:
        {
            auto* state = GetState(hWnd);
            if (state)
            {
                POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                HandleMouseMove(hWnd, *state, pt);
            }
        }
        return 0;

        case WM_LBUTTONUP:
        {
            auto* state = GetState(hWnd);
            if (state)
            {
                POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                HandleLeftButtonUp(hWnd, *state, pt);
            }
        }
        return 0;

        case WM_RBUTTONDOWN:
        {
            auto* state = GetState(hWnd);
            if (state)
            {
                POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                HandleRightButtonDown(hWnd, *state, pt);
            }
        }
        return 0;

        case WM_KEYDOWN:
        {
            auto* state = GetState(hWnd);
            if (state && wParam == VK_DELETE)
            {
                DeleteSelection(hWnd, *state);
            }
        }
        return 0;

        case WM_PAINT:
        {
            auto* state = GetState(hWnd);
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hWnd, &ps);
            if (state)
            {
                EditorRenderer::Draw(hWnd, hdc, *state);
            }
            EndPaint(hWnd, &ps);
        }
        return 0;

        case WM_DESTROY:
        {
            auto* state = GetState(hWnd);
            delete state;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
            PostQuitMessage(0);
        }
        return 0;
        }

        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
}
