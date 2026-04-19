#pragma once
#include "../Core/LevelEditorState.h"
#include <windows.h>

namespace Figure::Editor::LevelEditor
{
    class LevelEditorWindow
    {
    public:
        static bool Register(HINSTANCE hInstance);
        static HWND Create(HINSTANCE hInstance, int nCmdShow);

    private:
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        static LevelEditorState* GetState(HWND hWnd);
        static void HandleCommand(HWND hWnd, LevelEditorState& state, int commandId);
        static void HandleLeftButtonDown(HWND hWnd, LevelEditorState& state, POINT point);
        static void HandleMouseMove(HWND hWnd, LevelEditorState& state, POINT point);
        static void HandleLeftButtonUp(HWND hWnd, LevelEditorState& state, POINT point);
        static void HandleRightButtonDown(HWND hWnd, LevelEditorState& state, POINT point);
        static bool SaveCurrentMap(HWND hWnd, LevelEditorState& state, bool saveAs);
        static void DeleteSelection(HWND hWnd, LevelEditorState& state);
    };
}
