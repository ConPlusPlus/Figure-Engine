// Figure Engine.cpp : Defines the entry point for the application.
// This folder has turned into more of the launcher implementation, but the name stuck. It is what it is.

#include "framework.h"
#include "Figure Engine.h"
#include <string>
#include <vector>
#include <algorithm>
#include <shellapi.h>
#include <commdlg.h>
#include <windowsx.h>

#define MAX_LOADSTRING 100

enum class LauncherPage
{
    Home,
    Projects,
    Build,
    Run,
    Settings,
    Tools,
    OutputLog
};

struct ProjectConfig
{
    std::wstring projectFilePath;
    std::wstring projectName;
    std::wstring startupMap;
    std::wstring gameExecutable;
    std::wstring buildDebugCommand;
    std::wstring buildReleaseCommand;
    std::wstring packageCommand;
    std::wstring levelEditorExecutable;
    std::wstring animationEditorExecutable;
    std::wstring materialEditorExecutable;
    std::wstring shaderEditorExecutable;
    std::wstring uiEditorExecutable;
    std::wstring audioEditorExecutable;
    std::wstring vfxEditorExecutable;
};

// Global Variables:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
const wchar_t LEVEL_EDITOR_WINDOW_CLASS[] = L"FigureIntegratedLevelEditorWindow";

LauncherPage g_CurrentPage = LauncherPage::Home;
ProjectConfig g_Project;
std::vector<std::wstring> g_RecentProjects;
std::vector<std::wstring> g_OutputLog =
{
    L"Figure Engine launcher started."
};

const int SIDEBAR_WIDTH = 220;
const int LOG_HEIGHT = 160;
const int NAV_BUTTON_HEIGHT = 44;
const int NAV_BUTTON_MARGIN = 8;

// Forward declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                RegisterLevelEditorClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
HWND                CreateLevelEditorWindow(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    LevelEditorWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void SetLauncherPage(LauncherPage page, HWND hWnd);
void AddLog(const std::wstring& text, HWND hWnd);
void DrawLauncher(HWND hWnd, HDC hdc);
void DrawSidebar(HDC hdc, RECT clientRect);
void DrawMainContent(HDC hdc, RECT clientRect);
void DrawBottomLog(HDC hdc, RECT clientRect);
void DrawHomePage(HDC hdc, RECT rc);
void DrawProjectsPage(HDC hdc, RECT rc);
void DrawBuildPage(HDC hdc, RECT rc);
void DrawRunPage(HDC hdc, RECT rc);
void DrawSettingsPage(HDC hdc, RECT rc);
void DrawToolsPage(HDC hdc, RECT rc);
void DrawOutputLogPage(HDC hdc, RECT rc);
void DrawTextLine(HDC hdc, int x, int y, const std::wstring& text);
void DrawSectionTitle(HDC hdc, int x, int y, const std::wstring& text);
std::wstring DisplayValue(const std::wstring& value, const std::wstring& fallback = L"<not set>");

std::wstring GetAppDirectory();
std::wstring GetLauncherConfigPath();
std::wstring Trim(const std::wstring& text);
std::wstring FileNameWithoutExtension(const std::wstring& path);
std::wstring DirectoryFromPath(const std::wstring& path);
std::wstring ReadIniString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue, const std::wstring& path);
void WriteIniString(const std::wstring& section, const std::wstring& key, const std::wstring& value, const std::wstring& path);
void LoadRecentProjects();
void SaveRecentProjects();
void AddRecentProject(const std::wstring& projectPath);
bool LoadProjectConfig(const std::wstring& projectPath);
bool SaveProjectConfig();
std::wstring BrowseForOpenPath(HWND hWnd, const wchar_t* filter, const wchar_t* title);
std::wstring BrowseForSavePath(HWND hWnd, const wchar_t* filter, const wchar_t* title, const wchar_t* defaultExt);
void CreateNewProject(HWND hWnd);
void OpenProject(HWND hWnd);
bool EnsureCurrentProject(HWND hWnd);
bool EnsureExecutablePath(HWND hWnd, std::wstring& targetPath, const std::wstring& promptTitle);
bool ExecuteProcess(const std::wstring& commandLine, const std::wstring& workingDirectory, HWND hWnd, const std::wstring& successText);
bool LaunchExecutable(const std::wstring& executablePath, const std::wstring& arguments, HWND hWnd, const std::wstring& successText);
void RunGame(HWND hWnd, bool debugRun);
void RunConfiguredCommand(HWND hWnd, const std::wstring& commandLine, const std::wstring& displayName);
void LaunchTool(HWND hWnd, std::wstring& toolPath, const std::wstring& title, const std::wstring& successText);
LauncherPage PageFromPoint(POINT pt, RECT clientRect);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FIGUREENGINE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    RegisterLevelEditorClass(hInstance);
    LoadRecentProjects();

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    if (!g_RecentProjects.empty())
    {
        if (LoadProjectConfig(g_RecentProjects.front()))
        {
            AddLog(L"Loaded most recent project on startup.", nullptr);
        }
    }
    else
    {
        AddLog(L"No recent projects found.", nullptr);
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FIGUREENGINE));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIGUREENGINE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FIGUREENGINE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM RegisterLevelEditorClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = LevelEditorWndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIGUREENGINE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = LEVEL_EDITOR_WINDOW_CLASS;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

HWND CreateLevelEditorWindow(HINSTANCE hInstance)
{
    return CreateWindowW(
        LEVEL_EDITOR_WINDOW_CLASS,
        L"Figure Engine Level Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        1280, 820,
        nullptr, nullptr,
        hInstance, nullptr);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        1360, 860,
        nullptr, nullptr,
        hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void SetLauncherPage(LauncherPage page, HWND hWnd)
{
    g_CurrentPage = page;
    if (hWnd)
    {
        InvalidateRect(hWnd, nullptr, TRUE);
    }
}

void AddLog(const std::wstring& text, HWND hWnd)
{
    g_OutputLog.push_back(text);

    if (g_OutputLog.size() > 250)
    {
        g_OutputLog.erase(g_OutputLog.begin(), g_OutputLog.begin() + 50);
    }

    if (hWnd)
    {
        InvalidateRect(hWnd, nullptr, TRUE);
    }
}

void DrawTextLine(HDC hdc, int x, int y, const std::wstring& text)
{
    TextOutW(hdc, x, y, text.c_str(), (int)text.length());
}

void DrawSectionTitle(HDC hdc, int x, int y, const std::wstring& text)
{
    DrawTextLine(hdc, x, y, text);
}

std::wstring DisplayValue(const std::wstring& value, const std::wstring& fallback)
{
    return value.empty() ? fallback : value;
}

void FillSolidRect(HDC hdc, RECT rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void DrawSidebarButton(HDC hdc, RECT rc, const std::wstring& label, bool active)
{
    FillSolidRect(hdc, rc, active ? RGB(70, 110, 180) : RGB(235, 235, 235));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, active ? RGB(255, 255, 255) : RGB(20, 20, 20));

    RECT textRc = rc;
    textRc.left += 12;
    DrawTextW(hdc, label.c_str(), -1, &textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void DrawSidebar(HDC hdc, RECT clientRect)
{
    RECT sidebar = { 0, 0, SIDEBAR_WIDTH, clientRect.bottom - LOG_HEIGHT };
    FillSolidRect(hdc, sidebar, RGB(245, 245, 245));

    SetTextColor(hdc, RGB(20, 20, 20));
    SetBkMode(hdc, TRANSPARENT);
    DrawTextLine(hdc, 18, 18, L"Figure Engine");
    DrawTextLine(hdc, 18, 42, L"Launcher");

    struct NavItem
    {
        LauncherPage page;
        const wchar_t* label;
    };

    NavItem items[] =
    {
        { LauncherPage::Home,      L"Home" },
        { LauncherPage::Projects,  L"Projects" },
        { LauncherPage::Build,     L"Build" },
        { LauncherPage::Run,       L"Run" },
        { LauncherPage::Settings,  L"Settings" },
        { LauncherPage::Tools,     L"Tools" },
        { LauncherPage::OutputLog, L"Output Log" }
    };

    int y = 80;
    for (const auto& item : items)
    {
        RECT btn = { 12, y, SIDEBAR_WIDTH - 12, y + NAV_BUTTON_HEIGHT };
        DrawSidebarButton(hdc, btn, item.label, g_CurrentPage == item.page);
        y += NAV_BUTTON_HEIGHT + NAV_BUTTON_MARGIN;
    }
}

void DrawHomePage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Home");
    y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Current Project: " + DisplayValue(g_Project.projectName, L"<none loaded>"));
    y += 22;
    DrawTextLine(hdc, rc.left + 20, y, L"Project File: " + DisplayValue(g_Project.projectFilePath));
    y += 32;
    DrawTextLine(hdc, rc.left + 20, y, L"Quick Overview");
    y += 22;
    DrawTextLine(hdc, rc.left + 40, y, L"- Open or create a .figproj project");
    y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"- Run the configured game executable");
    y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"- Build/package through project-defined commands");
    y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"- Launch tools like the level editor from here");
    y += 30;
    DrawTextLine(hdc, rc.left + 20, y, L"Recent Projects: " + std::to_wstring(g_RecentProjects.size()));
}

void DrawProjectsPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Projects");
    y += 36;

    DrawTextLine(hdc, rc.left + 20, y, L"Loaded Project");
    y += 22;
    DrawTextLine(hdc, rc.left + 40, y, L"Name: " + DisplayValue(g_Project.projectName));
    y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"Startup Map: " + DisplayValue(g_Project.startupMap));
    y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"File: " + DisplayValue(g_Project.projectFilePath));
    y += 32;

    DrawTextLine(hdc, rc.left + 20, y, L"Recent Projects");
    y += 22;

    if (g_RecentProjects.empty())
    {
        DrawTextLine(hdc, rc.left + 40, y, L"No recent projects.");
        return;
    }

    for (size_t i = 0; i < g_RecentProjects.size() && i < 8; ++i)
    {
        DrawTextLine(hdc, rc.left + 40, y, std::to_wstring(i + 1) + L". " + g_RecentProjects[i]);
        y += 20;
    }
}

void DrawBuildPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Build");
    y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Build Debug Command: " + DisplayValue(g_Project.buildDebugCommand));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Build Release Command: " + DisplayValue(g_Project.buildReleaseCommand));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Package Command: " + DisplayValue(g_Project.packageCommand));
    y += 30;
    DrawTextLine(hdc, rc.left + 20, y, L"These commands are read from the current .figproj file.");
}

void DrawRunPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Run");
    y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Game Executable: " + DisplayValue(g_Project.gameExecutable));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Startup Map: " + DisplayValue(g_Project.startupMap));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Run Game passes: -project \"<path>\"");
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Run Debug passes: -project \"<path>\" -debug");
}

void DrawSettingsPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Settings");
    y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Launcher Config: " + GetLauncherConfigPath());
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"App Directory: " + GetAppDirectory());
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Project Name: " + DisplayValue(g_Project.projectName));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Startup Map: " + DisplayValue(g_Project.startupMap));
    y += 30;
    DrawTextLine(hdc, rc.left + 20, y, L"Save Settings writes the current config back into the .figproj file.");
}

void DrawToolsPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Tools");
    y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Level Editor: " + DisplayValue(g_Project.levelEditorExecutable));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Animation Editor: " + DisplayValue(g_Project.animationEditorExecutable));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Material Editor: " + DisplayValue(g_Project.materialEditorExecutable));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Shader Editor: " + DisplayValue(g_Project.shaderEditorExecutable));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"UI Editor: " + DisplayValue(g_Project.uiEditorExecutable));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Audio Editor: " + DisplayValue(g_Project.audioEditorExecutable));
    y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"VFX Editor: " + DisplayValue(g_Project.vfxEditorExecutable));
}

void DrawOutputLogPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Output Log");
    y += 36;

    size_t start = 0;
    if (g_OutputLog.size() > 20)
    {
        start = g_OutputLog.size() - 20;
    }

    for (size_t i = start; i < g_OutputLog.size(); ++i)
    {
        DrawTextLine(hdc, rc.left + 20, y, g_OutputLog[i]);
        y += 20;
    }
}

void DrawMainContent(HDC hdc, RECT clientRect)
{
    RECT mainRc =
    {
        SIDEBAR_WIDTH,
        0,
        clientRect.right,
        clientRect.bottom - LOG_HEIGHT
    };

    FillSolidRect(hdc, mainRc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(20, 20, 20));

    switch (g_CurrentPage)
    {
    case LauncherPage::Home:
        DrawHomePage(hdc, mainRc);
        break;
    case LauncherPage::Projects:
        DrawProjectsPage(hdc, mainRc);
        break;
    case LauncherPage::Build:
        DrawBuildPage(hdc, mainRc);
        break;
    case LauncherPage::Run:
        DrawRunPage(hdc, mainRc);
        break;
    case LauncherPage::Settings:
        DrawSettingsPage(hdc, mainRc);
        break;
    case LauncherPage::Tools:
        DrawToolsPage(hdc, mainRc);
        break;
    case LauncherPage::OutputLog:
        DrawOutputLogPage(hdc, mainRc);
        break;
    }
}

void DrawBottomLog(HDC hdc, RECT clientRect)
{
    RECT logRc =
    {
        0,
        clientRect.bottom - LOG_HEIGHT,
        clientRect.right,
        clientRect.bottom
    };

    FillSolidRect(hdc, logRc, RGB(248, 248, 248));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(20, 20, 20));

    DrawTextLine(hdc, 12, logRc.top + 10, L"Launcher Log");

    int y = logRc.top + 34;
    size_t start = 0;
    if (g_OutputLog.size() > 5)
    {
        start = g_OutputLog.size() - 5;
    }

    for (size_t i = start; i < g_OutputLog.size(); ++i)
    {
        DrawTextLine(hdc, 12, y, g_OutputLog[i]);
        y += 22;
    }
}

void DrawLauncher(HWND hWnd, HDC hdc)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    DrawSidebar(hdc, clientRect);
    DrawMainContent(hdc, clientRect);
    DrawBottomLog(hdc, clientRect);
}

LauncherPage PageFromPoint(POINT pt, RECT clientRect)
{
    if (pt.x > SIDEBAR_WIDTH || pt.y > clientRect.bottom - LOG_HEIGHT)
    {
        return g_CurrentPage;
    }

    int y = 80;
    LauncherPage pages[] =
    {
        LauncherPage::Home,
        LauncherPage::Projects,
        LauncherPage::Build,
        LauncherPage::Run,
        LauncherPage::Settings,
        LauncherPage::Tools,
        LauncherPage::OutputLog
    };

    for (LauncherPage page : pages)
    {
        RECT btn = { 12, y, SIDEBAR_WIDTH - 12, y + NAV_BUTTON_HEIGHT };
        if (PtInRect(&btn, pt))
        {
            return page;
        }
        y += NAV_BUTTON_HEIGHT + NAV_BUTTON_MARGIN;
    }

    return g_CurrentPage;
}

std::wstring GetAppDirectory()
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring fullPath(path);
    size_t slashPos = fullPath.find_last_of(L"\\/");
    return slashPos == std::wstring::npos ? L"." : fullPath.substr(0, slashPos);
}

std::wstring GetLauncherConfigPath()
{
    return GetAppDirectory() + L"\\launcher.ini";
}

std::wstring Trim(const std::wstring& text)
{
    size_t start = text.find_first_not_of(L" \t\r\n");
    if (start == std::wstring::npos)
    {
        return L"";
    }

    size_t end = text.find_last_not_of(L" \t\r\n");
    return text.substr(start, end - start + 1);
}

std::wstring FileNameWithoutExtension(const std::wstring& path)
{
    size_t slashPos = path.find_last_of(L"\\/");
    std::wstring fileName = (slashPos == std::wstring::npos) ? path : path.substr(slashPos + 1);
    size_t dotPos = fileName.find_last_of(L'.');
    return (dotPos == std::wstring::npos) ? fileName : fileName.substr(0, dotPos);
}

std::wstring DirectoryFromPath(const std::wstring& path)
{
    size_t slashPos = path.find_last_of(L"\\/");
    return slashPos == std::wstring::npos ? GetAppDirectory() : path.substr(0, slashPos);
}

std::wstring ReadIniString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue, const std::wstring& path)
{
    wchar_t buffer[2048] = {};
    GetPrivateProfileStringW(section.c_str(), key.c_str(), defaultValue.c_str(), buffer, 2048, path.c_str());
    return buffer;
}

void WriteIniString(const std::wstring& section, const std::wstring& key, const std::wstring& value, const std::wstring& path)
{
    WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), path.c_str());
}

void LoadRecentProjects()
{
    g_RecentProjects.clear();
    std::wstring configPath = GetLauncherConfigPath();

    for (int i = 0; i < 10; ++i)
    {
        std::wstring key = L"Project" + std::to_wstring(i);
        std::wstring value = Trim(ReadIniString(L"RecentProjects", key, L"", configPath));
        if (!value.empty())
        {
            g_RecentProjects.push_back(value);
        }
    }
}

void SaveRecentProjects()
{
    std::wstring configPath = GetLauncherConfigPath();

    for (int i = 0; i < 10; ++i)
    {
        std::wstring key = L"Project" + std::to_wstring(i);
        std::wstring value = (i < (int)g_RecentProjects.size()) ? g_RecentProjects[i] : L"";
        WriteIniString(L"RecentProjects", key, value, configPath);
    }
}

void AddRecentProject(const std::wstring& projectPath)
{
    g_RecentProjects.erase(std::remove(g_RecentProjects.begin(), g_RecentProjects.end(), projectPath), g_RecentProjects.end());
    g_RecentProjects.insert(g_RecentProjects.begin(), projectPath);

    if (g_RecentProjects.size() > 10)
    {
        g_RecentProjects.resize(10);
    }

    SaveRecentProjects();
}

bool LoadProjectConfig(const std::wstring& projectPath)
{
    g_Project = {};
    g_Project.projectFilePath = projectPath;
    g_Project.projectName = ReadIniString(L"Project", L"Name", FileNameWithoutExtension(projectPath), projectPath);
    g_Project.startupMap = ReadIniString(L"Project", L"StartupMap", L"", projectPath);
    g_Project.gameExecutable = ReadIniString(L"Run", L"GameExecutable", L"", projectPath);
    g_Project.buildDebugCommand = ReadIniString(L"Build", L"DebugCommand", L"", projectPath);
    g_Project.buildReleaseCommand = ReadIniString(L"Build", L"ReleaseCommand", L"", projectPath);
    g_Project.packageCommand = ReadIniString(L"Build", L"PackageCommand", L"", projectPath);
    g_Project.levelEditorExecutable = ReadIniString(L"Tools", L"LevelEditorExecutable", L"", projectPath);
    g_Project.animationEditorExecutable = ReadIniString(L"Tools", L"AnimationEditorExecutable", L"", projectPath);
    g_Project.materialEditorExecutable = ReadIniString(L"Tools", L"MaterialEditorExecutable", L"", projectPath);
    g_Project.shaderEditorExecutable = ReadIniString(L"Tools", L"ShaderEditorExecutable", L"", projectPath);
    g_Project.uiEditorExecutable = ReadIniString(L"Tools", L"UIEditorExecutable", L"", projectPath);
    g_Project.audioEditorExecutable = ReadIniString(L"Tools", L"AudioEditorExecutable", L"", projectPath);
    g_Project.vfxEditorExecutable = ReadIniString(L"Tools", L"VfxEditorExecutable", L"", projectPath);

    AddRecentProject(projectPath);
    return true;
}

bool SaveProjectConfig()
{
    if (g_Project.projectFilePath.empty())
    {
        return false;
    }

    WriteIniString(L"Project", L"Name", g_Project.projectName, g_Project.projectFilePath);
    WriteIniString(L"Project", L"StartupMap", g_Project.startupMap, g_Project.projectFilePath);
    WriteIniString(L"Run", L"GameExecutable", g_Project.gameExecutable, g_Project.projectFilePath);
    WriteIniString(L"Build", L"DebugCommand", g_Project.buildDebugCommand, g_Project.projectFilePath);
    WriteIniString(L"Build", L"ReleaseCommand", g_Project.buildReleaseCommand, g_Project.projectFilePath);
    WriteIniString(L"Build", L"PackageCommand", g_Project.packageCommand, g_Project.projectFilePath);
    WriteIniString(L"Tools", L"LevelEditorExecutable", g_Project.levelEditorExecutable, g_Project.projectFilePath);
    WriteIniString(L"Tools", L"AnimationEditorExecutable", g_Project.animationEditorExecutable, g_Project.projectFilePath);
    WriteIniString(L"Tools", L"MaterialEditorExecutable", g_Project.materialEditorExecutable, g_Project.projectFilePath);
    WriteIniString(L"Tools", L"ShaderEditorExecutable", g_Project.shaderEditorExecutable, g_Project.projectFilePath);
    WriteIniString(L"Tools", L"UIEditorExecutable", g_Project.uiEditorExecutable, g_Project.projectFilePath);
    WriteIniString(L"Tools", L"AudioEditorExecutable", g_Project.audioEditorExecutable, g_Project.projectFilePath);
    WriteIniString(L"Tools", L"VfxEditorExecutable", g_Project.vfxEditorExecutable, g_Project.projectFilePath);
    AddRecentProject(g_Project.projectFilePath);
    return true;
}

std::wstring BrowseForOpenPath(HWND hWnd, const wchar_t* filter, const wchar_t* title)
{
    wchar_t fileBuffer[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = fileBuffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn))
    {
        return fileBuffer;
    }

    return L"";
}

std::wstring BrowseForSavePath(HWND hWnd, const wchar_t* filter, const wchar_t* title, const wchar_t* defaultExt)
{
    wchar_t fileBuffer[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = fileBuffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.lpstrTitle = title;
    ofn.lpstrDefExt = defaultExt;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn))
    {
        return fileBuffer;
    }

    return L"";
}

void CreateNewProject(HWND hWnd)
{
    std::wstring projectPath = BrowseForSavePath(hWnd, L"Figure Project (*.figproj)\0*.figproj\0All Files (*.*)\0*.*\0", L"Create Figure Project", L"figproj");
    if (projectPath.empty())
    {
        return;
    }

    g_Project = {};
    g_Project.projectFilePath = projectPath;
    g_Project.projectName = FileNameWithoutExtension(projectPath);
    g_Project.startupMap = L"Maps/Main.figmap";

    if (SaveProjectConfig())
    {
        AddLog(L"Created project: " + projectPath, hWnd);
        SetLauncherPage(LauncherPage::Projects, hWnd);
    }
}

void OpenProject(HWND hWnd)
{
    std::wstring projectPath = BrowseForOpenPath(hWnd, L"Figure Project (*.figproj)\0*.figproj\0All Files (*.*)\0*.*\0", L"Open Figure Project");
    if (projectPath.empty())
    {
        return;
    }

    if (LoadProjectConfig(projectPath))
    {
        AddLog(L"Opened project: " + projectPath, hWnd);
        SetLauncherPage(LauncherPage::Home, hWnd);
    }
}

bool EnsureCurrentProject(HWND hWnd)
{
    if (!g_Project.projectFilePath.empty())
    {
        return true;
    }

    MessageBoxW(hWnd, L"No project is currently loaded. Open or create a project first.", L"Figure Engine", MB_OK | MB_ICONWARNING);
    return false;
}

bool EnsureExecutablePath(HWND hWnd, std::wstring& targetPath, const std::wstring& promptTitle)
{
    if (!targetPath.empty())
    {
        return true;
    }

    targetPath = BrowseForOpenPath(hWnd, L"Executable (*.exe)\0*.exe\0All Files (*.*)\0*.*\0", promptTitle.c_str());
    if (targetPath.empty())
    {
        return false;
    }

    SaveProjectConfig();
    return true;
}

bool ExecuteProcess(const std::wstring& commandLine, const std::wstring& workingDirectory, HWND hWnd, const std::wstring& successText)
{
    std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back(L'\0');

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL created = CreateProcessW(
        nullptr,
        mutableCommand.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        workingDirectory.empty() ? nullptr : workingDirectory.c_str(),
        &si,
        &pi);

    if (!created)
    {
        std::wstring message = L"Failed to launch process.\n\nCommand:\n" + commandLine;
        MessageBoxW(hWnd, message.c_str(), L"Figure Engine", MB_OK | MB_ICONERROR);
        return false;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    AddLog(successText, hWnd);
    return true;
}

bool LaunchExecutable(const std::wstring& executablePath, const std::wstring& arguments, HWND hWnd, const std::wstring& successText)
{
    std::wstring command = L"\"" + executablePath + L"\"";
    if (!arguments.empty())
    {
        command += L" " + arguments;
    }

    return ExecuteProcess(command, DirectoryFromPath(executablePath), hWnd, successText);
}

void RunGame(HWND hWnd, bool debugRun)
{
    if (!EnsureCurrentProject(hWnd))
    {
        return;
    }

    if (!EnsureExecutablePath(hWnd, g_Project.gameExecutable, L"Select Game Executable"))
    {
        return;
    }

    std::wstring args = L"-project \"" + g_Project.projectFilePath + L"\"";
    if (debugRun)
    {
        args += L" -debug";
    }

    LaunchExecutable(g_Project.gameExecutable, args, hWnd, debugRun ? L"Started game in debug mode." : L"Started game.");
}

void RunConfiguredCommand(HWND hWnd, const std::wstring& commandLine, const std::wstring& displayName)
{
    if (!EnsureCurrentProject(hWnd))
    {
        return;
    }

    if (commandLine.empty())
    {
        std::wstring message = displayName + L" is not configured yet.\n\nAdd the command under the Build section in your .figproj file.";
        MessageBoxW(hWnd, message.c_str(), L"Figure Engine", MB_OK | MB_ICONINFORMATION);
        return;
    }

    std::wstring wrapped = L"cmd.exe /C " + commandLine;
    ExecuteProcess(wrapped, DirectoryFromPath(g_Project.projectFilePath), hWnd, displayName + L" started.");
}

void LaunchTool(HWND hWnd, std::wstring& toolPath, const std::wstring& title, const std::wstring& successText)
{
    if (!EnsureCurrentProject(hWnd))
    {
        return;
    }

    if (!EnsureExecutablePath(hWnd, toolPath, title))
    {
        return;
    }

    SaveProjectConfig();
    LaunchExecutable(toolPath, L"-project \"" + g_Project.projectFilePath + L"\"", hWnd, successText);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case IDM_NEW_PROJECT:
            CreateNewProject(hWnd);
            break;

        case IDM_OPEN_PROJECT:
            OpenProject(hWnd);
            break;

        case IDM_SAVE_SETTINGS:
            if (!EnsureCurrentProject(hWnd))
            {
                break;
            }
            if (SaveProjectConfig())
            {
                AddLog(L"Saved project settings.", hWnd);
                MessageBoxW(hWnd, L"Project settings saved to the .figproj file.", L"Figure Engine", MB_OK | MB_ICONINFORMATION);
            }
            break;

        case IDM_BUILD_DEBUG:
            RunConfiguredCommand(hWnd, g_Project.buildDebugCommand, L"Build Debug");
            SetLauncherPage(LauncherPage::Build, hWnd);
            break;

        case IDM_BUILD_RELEASE:
            RunConfiguredCommand(hWnd, g_Project.buildReleaseCommand, L"Build Release");
            SetLauncherPage(LauncherPage::Build, hWnd);
            break;

        case IDM_PACKAGE_GAME:
            RunConfiguredCommand(hWnd, g_Project.packageCommand, L"Package Game");
            SetLauncherPage(LauncherPage::Build, hWnd);
            break;

        case IDM_RUN_GAME:
            RunGame(hWnd, false);
            SetLauncherPage(LauncherPage::Run, hWnd);
            break;

        case IDM_RUN_DEBUG:
            RunGame(hWnd, true);
            SetLauncherPage(LauncherPage::Run, hWnd);
            break;

        case IDM_STOP_GAME:
            MessageBoxW(hWnd, L"Stop is not implemented yet. For now, close the launched process manually.", L"Figure Engine", MB_OK | MB_ICONINFORMATION);
            AddLog(L"Stop requested but not implemented yet.", hWnd);
            break;

        case IDM_TOOL_LEVEL_EDITOR:
        {
            int choice = MessageBoxW(
                hWnd,
                L"Yes = open the built-in Level Editor window\nNo = browse for a Level Editor executable\nCancel = do nothing",
                L"Open Level Editor",
                MB_YESNOCANCEL | MB_ICONQUESTION);

            if (choice == IDYES)
            {
                HWND hEditor = CreateLevelEditorWindow(hInst);
                if (hEditor)
                {
                    ShowWindow(hEditor, SW_SHOW);
                    UpdateWindow(hEditor);
                    AddLog(L"Opened built-in Level Editor window.", hWnd);
                }
                else
                {
                    MessageBoxW(hWnd, L"Failed to open the built-in Level Editor window.", L"Figure Engine", MB_OK | MB_ICONERROR);
                }
            }
            else if (choice == IDNO)
            {
                LaunchTool(hWnd, g_Project.levelEditorExecutable, L"Select Level Editor Executable", L"Started Level Editor.");
            }

            SetLauncherPage(LauncherPage::Tools, hWnd);
            break;
        }

        case IDM_TOOL_ANIMATION_EDITOR:
            LaunchTool(hWnd, g_Project.animationEditorExecutable, L"Select Animation Editor Executable", L"Started Animation Editor.");
            SetLauncherPage(LauncherPage::Tools, hWnd);
            break;

        case IDM_TOOL_MATERIAL_EDITOR:
            LaunchTool(hWnd, g_Project.materialEditorExecutable, L"Select Material Editor Executable", L"Started Material Editor.");
            SetLauncherPage(LauncherPage::Tools, hWnd);
            break;

        case IDM_TOOL_SHADER_EDITOR:
            LaunchTool(hWnd, g_Project.shaderEditorExecutable, L"Select Shader Editor Executable", L"Started Shader Editor.");
            SetLauncherPage(LauncherPage::Tools, hWnd);
            break;

        case IDM_TOOL_UI_EDITOR:
            LaunchTool(hWnd, g_Project.uiEditorExecutable, L"Select UI Editor Executable", L"Started UI Editor.");
            SetLauncherPage(LauncherPage::Tools, hWnd);
            break;

        case IDM_TOOL_AUDIO_EDITOR:
            LaunchTool(hWnd, g_Project.audioEditorExecutable, L"Select Audio Editor Executable", L"Started Audio Editor.");
            SetLauncherPage(LauncherPage::Tools, hWnd);
            break;

        case IDM_TOOL_VFX_EDITOR:
            LaunchTool(hWnd, g_Project.vfxEditorExecutable, L"Select VFX Editor Executable", L"Started VFX Editor.");
            SetLauncherPage(LauncherPage::Tools, hWnd);
            break;

        case IDM_VIEW_HOME:
            SetLauncherPage(LauncherPage::Home, hWnd);
            break;

        case IDM_VIEW_RECENT_PROJECTS:
            SetLauncherPage(LauncherPage::Projects, hWnd);
            break;

        case IDM_VIEW_PROJECT_SETTINGS:
            SetLauncherPage(LauncherPage::Settings, hWnd);
            break;

        case IDM_VIEW_ENGINE_SETTINGS:
            SetLauncherPage(LauncherPage::Settings, hWnd);
            break;

        case IDM_VIEW_BUILD_PACKAGE:
            SetLauncherPage(LauncherPage::Build, hWnd);
            break;

        case IDM_VIEW_RUN_TEST:
            SetLauncherPage(LauncherPage::Run, hWnd);
            break;

        case IDM_VIEW_TOOLS:
            SetLauncherPage(LauncherPage::Tools, hWnd);
            break;

        case IDM_VIEW_OUTPUT_LOG:
            SetLauncherPage(LauncherPage::OutputLog, hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_LBUTTONDOWN:
    {
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        LauncherPage clickedPage = PageFromPoint(pt, clientRect);
        if (clickedPage != g_CurrentPage)
        {
            SetLauncherPage(clickedPage, hWnd);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawLauncher(hWnd, hdc);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK LevelEditorWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        RECT toolbar = { 0, 0, clientRect.right, 52 };
        RECT tools = { 0, 52, 180, clientRect.bottom - 140 };
        RECT viewport = { 180, 52, clientRect.right - 280, clientRect.bottom - 140 };
        RECT properties = { clientRect.right - 280, 52, clientRect.right, clientRect.bottom - 140 };
        RECT output = { 0, clientRect.bottom - 140, clientRect.right, clientRect.bottom };

        FillSolidRect(hdc, toolbar, RGB(245, 245, 245));
        FillSolidRect(hdc, tools, RGB(248, 248, 248));
        FillSolidRect(hdc, viewport, RGB(255, 255, 255));
        FillSolidRect(hdc, properties, RGB(248, 248, 248));
        FillSolidRect(hdc, output, RGB(242, 242, 242));

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(20, 20, 20));

        DrawTextLine(hdc, 16, 18, L"Figure Engine Level Editor");
        DrawTextLine(hdc, 16, 72, L"Tools");
        DrawTextLine(hdc, 16, 102, L"- Select");
        DrawTextLine(hdc, 16, 122, L"- Block");
        DrawTextLine(hdc, 16, 142, L"- Entity");
        DrawTextLine(hdc, 196, 72, L"Viewport");
        DrawTextLine(hdc, 196, 102, L"Built-in editor window from the launcher.");
        DrawTextLine(hdc, 196, 122, L"You can replace this with your real LevelEditor code later.");
        DrawTextLine(hdc, clientRect.right - 260, 72, L"Properties");
        DrawTextLine(hdc, clientRect.right - 260, 102, L"Selection: None");
        DrawTextLine(hdc, 16, clientRect.bottom - 120, L"Output");
        DrawTextLine(hdc, 16, clientRect.bottom - 96, L"Launcher opened the built-in Level Editor.");

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }

    return (INT_PTR)FALSE;
}