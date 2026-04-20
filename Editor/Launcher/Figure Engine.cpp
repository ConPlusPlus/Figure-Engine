// Figure Engine.cpp : Defines the entry point for the launcher application.

#include "framework.h"
#include "Figure Engine.h"
#include <string>
#include <vector>
#include <algorithm>
#include <commdlg.h>
#include <windowsx.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

#define MAX_LOADSTRING 100

enum class LauncherPage
{
    Home,
    Projects,
    Build,
    Run,
    Settings,
    Tools,
    Updates,
    OutputLog
};

enum class EditorToolKind
{
    Level,
    Animation,
    Material,
    Shader,
    UI,
    Audio,
    VFX
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

struct UpdateEntry
{
    EditorToolKind Tool;
    std::wstring InstalledVersion;
    std::wstring AvailableVersion;
    std::wstring Title;
    std::wstring Notes;
    std::wstring DownloadUrl;
    bool HasFeedData = false;
};

struct ButtonDef
{
    RECT Rect = { 0,0,0,0 };
    std::wstring Label;
    int Id = 0;
};

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

LauncherPage g_CurrentPage = LauncherPage::Home;
ProjectConfig g_Project;
std::vector<std::wstring> g_RecentProjects;
std::vector<std::wstring> g_OutputLog = { L"Figure Engine launcher started." };
std::vector<ButtonDef> g_PageButtons;
std::vector<UpdateEntry> g_Updates;
std::wstring g_UpdateFeedSource;

const int SIDEBAR_WIDTH = 220;
const int LOG_HEIGHT = 160;
const int NAV_BUTTON_HEIGHT = 44;
const int NAV_BUTTON_MARGIN = 8;

enum ButtonIds
{
    BTN_SET_GAME_EXE = 1000,
    BTN_SET_LEVEL_EXE,
    BTN_SET_ANIMATION_EXE,
    BTN_SET_MATERIAL_EXE,
    BTN_SET_SHADER_EXE,
    BTN_SET_UI_EXE,
    BTN_SET_AUDIO_EXE,
    BTN_SET_VFX_EXE,
    BTN_REFRESH_UPDATES,
    BTN_DOWNLOAD_LEVEL,
    BTN_DOWNLOAD_ANIMATION,
    BTN_DOWNLOAD_MATERIAL,
    BTN_DOWNLOAD_SHADER,
    BTN_DOWNLOAD_UI,
    BTN_DOWNLOAD_AUDIO,
    BTN_DOWNLOAD_VFX,
    BTN_OPEN_UPDATES_FILE
};

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
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
void DrawUpdatesPage(HDC hdc, RECT rc);
void DrawOutputLogPage(HDC hdc, RECT rc);
void DrawTextLine(HDC hdc, int x, int y, const std::wstring& text);
void DrawSectionTitle(HDC hdc, int x, int y, const std::wstring& text);
void DrawButton(HDC hdc, const ButtonDef& button, bool emphasize = false);
std::wstring DisplayValue(const std::wstring& value, const std::wstring& fallback = L"<not set>");
std::wstring ToolDisplayName(EditorToolKind tool);
std::wstring* ToolPathRef(EditorToolKind tool);
LauncherPage PageFromPoint(POINT pt, RECT clientRect);
int HitTestButton(POINT pt);
void RebuildPageButtons(HWND hWnd);
void HandlePageButton(HWND hWnd, int buttonId);

std::wstring GetAppDirectory();
std::wstring GetLauncherConfigPath();
std::wstring GetDefaultUpdatesFeedPath();
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
void LaunchTool(HWND hWnd, EditorToolKind tool);

void LoadGlobalSettings();
void SaveGlobalSettings();
void LoadInstalledVersions();
void SaveInstalledVersion(EditorToolKind tool, const std::wstring& version);
UpdateEntry* FindUpdate(EditorToolKind tool);
bool RefreshUpdates(HWND hWnd);
bool DownloadUrlOrCopy(const std::wstring& source, const std::wstring& destination);
bool InstallToolUpdate(HWND hWnd, EditorToolKind tool);
std::wstring ToolSectionName(EditorToolKind tool);

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
    LoadGlobalSettings();
    LoadRecentProjects();

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    if (!g_RecentProjects.empty() && LoadProjectConfig(g_RecentProjects.front()))
    {
        AddLog(L"Loaded most recent project on startup.", nullptr);
    }

    RefreshUpdates(nullptr);

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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 1440, 900, nullptr, nullptr, hInstance, nullptr);
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
    if (hWnd) InvalidateRect(hWnd, nullptr, TRUE);
}

void AddLog(const std::wstring& text, HWND hWnd)
{
    g_OutputLog.push_back(text);
    if (g_OutputLog.size() > 250) g_OutputLog.erase(g_OutputLog.begin(), g_OutputLog.begin() + 50);
    if (hWnd) InvalidateRect(hWnd, nullptr, TRUE);
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
    RECT textRc = rc; textRc.left += 12;
    DrawTextW(hdc, label.c_str(), -1, &textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void DrawButton(HDC hdc, const ButtonDef& button, bool emphasize)
{
    FillSolidRect(hdc, button.Rect, emphasize ? RGB(70,110,180) : RGB(230,230,230));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, emphasize ? RGB(255,255,255) : RGB(20,20,20));
    DrawTextW(hdc, button.Label.c_str(), -1, const_cast<RECT*>(&button.Rect), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

std::wstring ToolDisplayName(EditorToolKind tool)
{
    switch (tool)
    {
    case EditorToolKind::Level: return L"Level Editor";
    case EditorToolKind::Animation: return L"Animation Editor";
    case EditorToolKind::Material: return L"Material Editor";
    case EditorToolKind::Shader: return L"Shader Editor";
    case EditorToolKind::UI: return L"UI Editor";
    case EditorToolKind::Audio: return L"Audio Editor";
    case EditorToolKind::VFX: return L"VFX Editor";
    }
    return L"Unknown Tool";
}

std::wstring ToolSectionName(EditorToolKind tool)
{
    switch (tool)
    {
    case EditorToolKind::Level: return L"LevelEditor";
    case EditorToolKind::Animation: return L"AnimationEditor";
    case EditorToolKind::Material: return L"MaterialEditor";
    case EditorToolKind::Shader: return L"ShaderEditor";
    case EditorToolKind::UI: return L"UIEditor";
    case EditorToolKind::Audio: return L"AudioEditor";
    case EditorToolKind::VFX: return L"VFXEditor";
    }
    return L"Unknown";
}

std::wstring* ToolPathRef(EditorToolKind tool)
{
    switch (tool)
    {
    case EditorToolKind::Level: return &g_Project.levelEditorExecutable;
    case EditorToolKind::Animation: return &g_Project.animationEditorExecutable;
    case EditorToolKind::Material: return &g_Project.materialEditorExecutable;
    case EditorToolKind::Shader: return &g_Project.shaderEditorExecutable;
    case EditorToolKind::UI: return &g_Project.uiEditorExecutable;
    case EditorToolKind::Audio: return &g_Project.audioEditorExecutable;
    case EditorToolKind::VFX: return &g_Project.vfxEditorExecutable;
    }
    return nullptr;
}

void DrawSidebar(HDC hdc, RECT clientRect)
{
    RECT sidebar = { 0, 0, SIDEBAR_WIDTH, clientRect.bottom - LOG_HEIGHT };
    FillSolidRect(hdc, sidebar, RGB(245, 245, 245));
    SetTextColor(hdc, RGB(20, 20, 20));
    SetBkMode(hdc, TRANSPARENT);
    DrawTextLine(hdc, 18, 18, L"Figure Engine");
    DrawTextLine(hdc, 18, 42, L"Launcher");

    struct NavItem { LauncherPage page; const wchar_t* label; };
    NavItem items[] = {
        { LauncherPage::Home,      L"Home" },
        { LauncherPage::Projects,  L"Projects" },
        { LauncherPage::Build,     L"Build" },
        { LauncherPage::Run,       L"Run" },
        { LauncherPage::Settings,  L"Settings" },
        { LauncherPage::Tools,     L"Tools" },
        { LauncherPage::Updates,   L"Updates" },
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
    DrawSectionTitle(hdc, rc.left + 20, y, L"Home"); y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Current Project: " + DisplayValue(g_Project.projectName, L"<none loaded>")); y += 22;
    DrawTextLine(hdc, rc.left + 20, y, L"Project File: " + DisplayValue(g_Project.projectFilePath)); y += 32;
    DrawTextLine(hdc, rc.left + 20, y, L"Standalone editor workflow"); y += 22;
    DrawTextLine(hdc, rc.left + 40, y, L"- Configure each editor executable under Settings"); y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"- Launch tools from the Tools menu"); y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"- Publish exe updates through updates.ini or a feed URL"); y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"- Download and replace editor executables from Updates");
}

void DrawProjectsPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Projects"); y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Loaded Project"); y += 22;
    DrawTextLine(hdc, rc.left + 40, y, L"Name: " + DisplayValue(g_Project.projectName)); y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"Startup Map: " + DisplayValue(g_Project.startupMap)); y += 20;
    DrawTextLine(hdc, rc.left + 40, y, L"File: " + DisplayValue(g_Project.projectFilePath)); y += 32;
    DrawTextLine(hdc, rc.left + 20, y, L"Recent Projects"); y += 22;
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
    DrawSectionTitle(hdc, rc.left + 20, y, L"Build"); y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Build Debug Command: " + DisplayValue(g_Project.buildDebugCommand)); y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Build Release Command: " + DisplayValue(g_Project.buildReleaseCommand)); y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Package Command: " + DisplayValue(g_Project.packageCommand)); y += 30;
    DrawTextLine(hdc, rc.left + 20, y, L"These commands are read from the current .figproj file.");
}

void DrawRunPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Run"); y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Game Executable: " + DisplayValue(g_Project.gameExecutable)); y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Startup Map: " + DisplayValue(g_Project.startupMap)); y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Run Game passes: -project \"<path>\""); y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Run Debug passes: -project \"<path>\" -debug");
}

void DrawSettingsPage(HDC hdc, RECT rc)
{
    g_PageButtons.clear();
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Settings"); y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Launcher Config: " + GetLauncherConfigPath()); y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Updates Feed: " + DisplayValue(g_UpdateFeedSource, GetDefaultUpdatesFeedPath())); y += 30;

    auto addToolSetting = [&](EditorToolKind tool, int buttonId)
    {
        std::wstring* pathRef = ToolPathRef(tool);
        DrawTextLine(hdc, rc.left + 20, y, ToolDisplayName(tool) + L": " + DisplayValue(pathRef ? *pathRef : L""));
        ButtonDef b{};
        b.Id = buttonId;
        b.Label = L"Browse";
        b.Rect = { rc.left + 760, y - 6, rc.left + 840, y + 20 };
        g_PageButtons.push_back(b);
        DrawButton(hdc, b);
        y += 34;
    };

    addToolSetting(EditorToolKind::Level, BTN_SET_LEVEL_EXE);
    addToolSetting(EditorToolKind::Animation, BTN_SET_ANIMATION_EXE);
    addToolSetting(EditorToolKind::Material, BTN_SET_MATERIAL_EXE);
    addToolSetting(EditorToolKind::Shader, BTN_SET_SHADER_EXE);
    addToolSetting(EditorToolKind::UI, BTN_SET_UI_EXE);
    addToolSetting(EditorToolKind::Audio, BTN_SET_AUDIO_EXE);
    addToolSetting(EditorToolKind::VFX, BTN_SET_VFX_EXE);

    DrawTextLine(hdc, rc.left + 20, y + 10, L"Place an updates.ini next to the launcher or set [Updater] FeedUrl in launcher.ini.");
}

void DrawToolsPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Tools"); y += 36;
    for (EditorToolKind tool : { EditorToolKind::Level, EditorToolKind::Animation, EditorToolKind::Material, EditorToolKind::Shader, EditorToolKind::UI, EditorToolKind::Audio, EditorToolKind::VFX })
    {
        UpdateEntry* update = FindUpdate(tool);
        std::wstring line = ToolDisplayName(tool) + L": " + DisplayValue(*ToolPathRef(tool));
        if (update && update->HasFeedData)
        {
            line += L" | Available: v" + update->AvailableVersion + L" - " + update->Title;
        }
        DrawTextLine(hdc, rc.left + 20, y, line);
        y += 22;
    }
    y += 18;
    DrawTextLine(hdc, rc.left + 20, y, L"Use the Tools menu to launch configured executables."); y += 20;
    DrawTextLine(hdc, rc.left + 20, y, L"Use Updates to download new editor exe builds.");
}

void DrawUpdatesPage(HDC hdc, RECT rc)
{
    g_PageButtons.clear();
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Updates"); y += 36;
    DrawTextLine(hdc, rc.left + 20, y, L"Feed Source: " + DisplayValue(g_UpdateFeedSource, GetDefaultUpdatesFeedPath()));
    ButtonDef refresh{}; refresh.Id = BTN_REFRESH_UPDATES; refresh.Label = L"Refresh"; refresh.Rect = { rc.left + 760, y - 6, rc.left + 860, y + 20 };
    ButtonDef openFeed{}; openFeed.Id = BTN_OPEN_UPDATES_FILE; openFeed.Label = L"Open Feed Folder"; openFeed.Rect = { rc.left + 870, y - 6, rc.left + 1010, y + 20 };
    g_PageButtons.push_back(refresh); g_PageButtons.push_back(openFeed);
    DrawButton(hdc, refresh, true);
    DrawButton(hdc, openFeed);
    y += 38;

    auto addUpdateRow = [&](EditorToolKind tool, int buttonId)
    {
        UpdateEntry* update = FindUpdate(tool);
        std::wstring installed = update ? DisplayValue(update->InstalledVersion, L"unknown") : L"unknown";
        std::wstring available = update ? DisplayValue(update->AvailableVersion, L"<none>") : L"<none>";
        std::wstring title = (update && !update->Title.empty()) ? update->Title : L"No update published";
        std::wstring notes = (update && !update->Notes.empty()) ? update->Notes : L"No notes available";

        DrawTextLine(hdc, rc.left + 20, y, ToolDisplayName(tool)); y += 20;
        DrawTextLine(hdc, rc.left + 40, y, L"Installed: v" + installed + L" | Available: v" + available); y += 20;
        DrawTextLine(hdc, rc.left + 40, y, L"Title: " + title); y += 20;
        DrawTextLine(hdc, rc.left + 40, y, L"Notes: " + notes);
        ButtonDef b{}; b.Id = buttonId; b.Label = L"Download"; b.Rect = { rc.left + 760, y - 10, rc.left + 860, y + 16 };
        g_PageButtons.push_back(b);
        DrawButton(hdc, b, update && update->HasFeedData && !update->DownloadUrl.empty());
        y += 34;
    };

    addUpdateRow(EditorToolKind::Level, BTN_DOWNLOAD_LEVEL);
    addUpdateRow(EditorToolKind::Animation, BTN_DOWNLOAD_ANIMATION);
    addUpdateRow(EditorToolKind::Material, BTN_DOWNLOAD_MATERIAL);
    addUpdateRow(EditorToolKind::Shader, BTN_DOWNLOAD_SHADER);
    addUpdateRow(EditorToolKind::UI, BTN_DOWNLOAD_UI);
    addUpdateRow(EditorToolKind::Audio, BTN_DOWNLOAD_AUDIO);
    addUpdateRow(EditorToolKind::VFX, BTN_DOWNLOAD_VFX);
}

void DrawOutputLogPage(HDC hdc, RECT rc)
{
    int y = rc.top + 20;
    DrawSectionTitle(hdc, rc.left + 20, y, L"Output Log"); y += 36;
    size_t start = g_OutputLog.size() > 20 ? g_OutputLog.size() - 20 : 0;
    for (size_t i = start; i < g_OutputLog.size(); ++i)
    {
        DrawTextLine(hdc, rc.left + 20, y, g_OutputLog[i]);
        y += 20;
    }
}

void DrawMainContent(HDC hdc, RECT clientRect)
{
    RECT mainRc = { SIDEBAR_WIDTH, 0, clientRect.right, clientRect.bottom - LOG_HEIGHT };
    FillSolidRect(hdc, mainRc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(20, 20, 20));

    switch (g_CurrentPage)
    {
    case LauncherPage::Home: DrawHomePage(hdc, mainRc); break;
    case LauncherPage::Projects: DrawProjectsPage(hdc, mainRc); break;
    case LauncherPage::Build: DrawBuildPage(hdc, mainRc); break;
    case LauncherPage::Run: DrawRunPage(hdc, mainRc); break;
    case LauncherPage::Settings: DrawSettingsPage(hdc, mainRc); break;
    case LauncherPage::Tools: DrawToolsPage(hdc, mainRc); break;
    case LauncherPage::Updates: DrawUpdatesPage(hdc, mainRc); break;
    case LauncherPage::OutputLog: DrawOutputLogPage(hdc, mainRc); break;
    }
}

void DrawBottomLog(HDC hdc, RECT clientRect)
{
    RECT logRc = { 0, clientRect.bottom - LOG_HEIGHT, clientRect.right, clientRect.bottom };
    FillSolidRect(hdc, logRc, RGB(248, 248, 248));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(20, 20, 20));
    DrawTextLine(hdc, 12, logRc.top + 10, L"Launcher Log");
    int y = logRc.top + 34;
    size_t start = g_OutputLog.size() > 5 ? g_OutputLog.size() - 5 : 0;
    for (size_t i = start; i < g_OutputLog.size(); ++i)
    {
        DrawTextLine(hdc, 12, y, g_OutputLog[i]);
        y += 22;
    }
}

void DrawLauncher(HWND hWnd, HDC hdc)
{
    RECT clientRect; GetClientRect(hWnd, &clientRect);
    DrawSidebar(hdc, clientRect);
    DrawMainContent(hdc, clientRect);
    DrawBottomLog(hdc, clientRect);
}

LauncherPage PageFromPoint(POINT pt, RECT clientRect)
{
    if (pt.x > SIDEBAR_WIDTH || pt.y > clientRect.bottom - LOG_HEIGHT) return g_CurrentPage;
    int y = 80;
    LauncherPage pages[] = { LauncherPage::Home, LauncherPage::Projects, LauncherPage::Build, LauncherPage::Run, LauncherPage::Settings, LauncherPage::Tools, LauncherPage::Updates, LauncherPage::OutputLog };
    for (LauncherPage page : pages)
    {
        RECT btn = { 12, y, SIDEBAR_WIDTH - 12, y + NAV_BUTTON_HEIGHT };
        if (PtInRect(&btn, pt)) return page;
        y += NAV_BUTTON_HEIGHT + NAV_BUTTON_MARGIN;
    }
    return g_CurrentPage;
}

int HitTestButton(POINT pt)
{
    for (const auto& button : g_PageButtons)
    {
        if (PtInRect(&button.Rect, pt)) return button.Id;
    }
    return 0;
}

std::wstring GetAppDirectory()
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring fullPath(path);
    size_t slashPos = fullPath.find_last_of(L"\\/");
    return slashPos == std::wstring::npos ? L"." : fullPath.substr(0, slashPos);
}

std::wstring GetLauncherConfigPath() { return GetAppDirectory() + L"\\launcher.ini"; }
std::wstring GetDefaultUpdatesFeedPath() { return GetAppDirectory() + L"\\updates.ini"; }

std::wstring Trim(const std::wstring& text)
{
    size_t start = text.find_first_not_of(L" \t\r\n");
    if (start == std::wstring::npos) return L"";
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
    wchar_t buffer[4096] = {};
    GetPrivateProfileStringW(section.c_str(), key.c_str(), defaultValue.c_str(), buffer, 4096, path.c_str());
    return buffer;
}

void WriteIniString(const std::wstring& section, const std::wstring& key, const std::wstring& value, const std::wstring& path)
{
    WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), path.c_str());
}

void LoadGlobalSettings()
{
    std::wstring config = GetLauncherConfigPath();
    g_UpdateFeedSource = Trim(ReadIniString(L"Updater", L"FeedUrl", L"", config));
    if (g_UpdateFeedSource.empty())
    {
        g_UpdateFeedSource = GetDefaultUpdatesFeedPath();
    }
}

void LoadRecentProjects()
{
    g_RecentProjects.clear();
    std::wstring configPath = GetLauncherConfigPath();
    for (int i = 0; i < 10; ++i)
    {
        std::wstring key = L"Project" + std::to_wstring(i);
        std::wstring value = Trim(ReadIniString(L"RecentProjects", key, L"", configPath));
        if (!value.empty()) g_RecentProjects.push_back(value);
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
    if (g_RecentProjects.size() > 10) g_RecentProjects.resize(10);
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
    LoadInstalledVersions();
    return true;
}

bool SaveProjectConfig()
{
    if (g_Project.projectFilePath.empty()) return false;
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

void LoadInstalledVersions()
{
    if (g_Project.projectFilePath.empty()) return;
    g_Updates.clear();
    for (EditorToolKind tool : { EditorToolKind::Level, EditorToolKind::Animation, EditorToolKind::Material, EditorToolKind::Shader, EditorToolKind::UI, EditorToolKind::Audio, EditorToolKind::VFX })
    {
        UpdateEntry e{};
        e.Tool = tool;
        e.InstalledVersion = ReadIniString(L"InstalledVersions", ToolSectionName(tool), L"0.0.0", g_Project.projectFilePath);
        g_Updates.push_back(e);
    }
}

void SaveInstalledVersion(EditorToolKind tool, const std::wstring& version)
{
    if (g_Project.projectFilePath.empty()) return;
    WriteIniString(L"InstalledVersions", ToolSectionName(tool), version, g_Project.projectFilePath);
    UpdateEntry* entry = FindUpdate(tool);
    if (entry) entry->InstalledVersion = version;
}

UpdateEntry* FindUpdate(EditorToolKind tool)
{
    for (auto& update : g_Updates) if (update.Tool == tool) return &update;
    return nullptr;
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
    return GetOpenFileNameW(&ofn) ? fileBuffer : L"";
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
    return GetSaveFileNameW(&ofn) ? fileBuffer : L"";
}

void CreateNewProject(HWND hWnd)
{
    std::wstring projectPath = BrowseForSavePath(hWnd, L"Figure Project (*.figproj)\0*.figproj\0All Files (*.*)\0*.*\0", L"Create Figure Project", L"figproj");
    if (projectPath.empty()) return;
    g_Project = {};
    g_Project.projectFilePath = projectPath;
    g_Project.projectName = FileNameWithoutExtension(projectPath);
    g_Project.startupMap = L"Maps/Main.figmap";
    if (SaveProjectConfig())
    {
        LoadInstalledVersions();
        AddLog(L"Created project: " + projectPath, hWnd);
        SetLauncherPage(LauncherPage::Projects, hWnd);
    }
}

void OpenProject(HWND hWnd)
{
    std::wstring projectPath = BrowseForOpenPath(hWnd, L"Figure Project (*.figproj)\0*.figproj\0All Files (*.*)\0*.*\0", L"Open Figure Project");
    if (projectPath.empty()) return;
    if (LoadProjectConfig(projectPath))
    {
        AddLog(L"Opened project: " + projectPath, hWnd);
        SetLauncherPage(LauncherPage::Home, hWnd);
    }
}

bool EnsureCurrentProject(HWND hWnd)
{
    if (!g_Project.projectFilePath.empty()) return true;
    MessageBoxW(hWnd, L"No project is currently loaded. Open or create a project first.", L"Figure Engine", MB_OK | MB_ICONWARNING);
    return false;
}

bool EnsureExecutablePath(HWND hWnd, std::wstring& targetPath, const std::wstring& promptTitle)
{
    if (!targetPath.empty()) return true;
    targetPath = BrowseForOpenPath(hWnd, L"Executable (*.exe)\0*.exe\0All Files (*.*)\0*.*\0", promptTitle.c_str());
    if (targetPath.empty()) return false;
    SaveProjectConfig();
    return true;
}

bool ExecuteProcess(const std::wstring& commandLine, const std::wstring& workingDirectory, HWND hWnd, const std::wstring& successText)
{
    std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back(L'\0');
    STARTUPINFOW si = {}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    BOOL created = CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr,
        workingDirectory.empty() ? nullptr : workingDirectory.c_str(), &si, &pi);
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
    if (!arguments.empty()) command += L" " + arguments;
    return ExecuteProcess(command, DirectoryFromPath(executablePath), hWnd, successText);
}

void RunGame(HWND hWnd, bool debugRun)
{
    if (!EnsureCurrentProject(hWnd)) return;
    if (!EnsureExecutablePath(hWnd, g_Project.gameExecutable, L"Select Game Executable")) return;
    std::wstring args = L"-project \"" + g_Project.projectFilePath + L"\"";
    if (debugRun) args += L" -debug";
    LaunchExecutable(g_Project.gameExecutable, args, hWnd, debugRun ? L"Started game in debug mode." : L"Started game.");
}

void RunConfiguredCommand(HWND hWnd, const std::wstring& commandLine, const std::wstring& displayName)
{
    if (!EnsureCurrentProject(hWnd)) return;
    if (commandLine.empty())
    {
        std::wstring message = displayName + L" is not configured yet.\n\nAdd the command under the Build section in your .figproj file.";
        MessageBoxW(hWnd, message.c_str(), L"Figure Engine", MB_OK | MB_ICONINFORMATION);
        return;
    }
    ExecuteProcess(L"cmd.exe /C " + commandLine, DirectoryFromPath(g_Project.projectFilePath), hWnd, displayName + L" started.");
}

void LaunchTool(HWND hWnd, EditorToolKind tool)
{
    if (!EnsureCurrentProject(hWnd)) return;
    std::wstring* pathRef = ToolPathRef(tool);
    if (!pathRef) return;
    if (!EnsureExecutablePath(hWnd, *pathRef, L"Select " + ToolDisplayName(tool) + L" Executable")) return;
    SaveProjectConfig();
    LaunchExecutable(*pathRef, L"-project \"" + g_Project.projectFilePath + L"\"", hWnd, L"Started " + ToolDisplayName(tool) + L".");
}

bool DownloadUrlOrCopy(const std::wstring& source, const std::wstring& destination)
{
    if (source.rfind(L"http://", 0) == 0 || source.rfind(L"https://", 0) == 0)
    {
        HRESULT hr = URLDownloadToFileW(nullptr, source.c_str(), destination.c_str(), 0, nullptr);
        return SUCCEEDED(hr);
    }
    return CopyFileW(source.c_str(), destination.c_str(), FALSE) == TRUE;
}

bool RefreshUpdates(HWND hWnd)
{
    if (!EnsureCurrentProject(hWnd) && hWnd != nullptr)
    {
        return false;
    }

    LoadInstalledVersions();

    std::wstring feedPath = g_UpdateFeedSource.empty() ? GetDefaultUpdatesFeedPath() : g_UpdateFeedSource;
    std::wstring localFeed = feedPath;
    bool downloaded = false;
    if (feedPath.rfind(L"http://", 0) == 0 || feedPath.rfind(L"https://", 0) == 0)
    {
        localFeed = GetAppDirectory() + L"\\_downloaded_updates.ini";
        downloaded = DownloadUrlOrCopy(feedPath, localFeed);
        if (!downloaded)
        {
            AddLog(L"Failed to download updates feed: " + feedPath, hWnd);
            return false;
        }
    }

    for (auto& update : g_Updates)
    {
        const std::wstring section = ToolSectionName(update.Tool);
        update.AvailableVersion = Trim(ReadIniString(section, L"Version", L"", localFeed));
        update.Title = Trim(ReadIniString(section, L"Title", L"", localFeed));
        update.Notes = Trim(ReadIniString(section, L"Notes", L"", localFeed));
        update.DownloadUrl = Trim(ReadIniString(section, L"DownloadUrl", L"", localFeed));
        update.HasFeedData = !update.AvailableVersion.empty() || !update.Title.empty() || !update.DownloadUrl.empty();
    }

    AddLog(L"Refreshed update feed.", hWnd);
    return true;
}

bool InstallToolUpdate(HWND hWnd, EditorToolKind tool)
{
    if (!EnsureCurrentProject(hWnd)) return false;
    UpdateEntry* update = FindUpdate(tool);
    std::wstring* targetPath = ToolPathRef(tool);
    if (!update || !targetPath)
    {
        MessageBoxW(hWnd, L"Update information for this tool could not be found.", L"Figure Engine", MB_OK | MB_ICONERROR);
        return false;
    }
    if (update->DownloadUrl.empty())
    {
        MessageBoxW(hWnd, L"No download URL is configured for this tool in the update feed.", L"Figure Engine", MB_OK | MB_ICONINFORMATION);
        return false;
    }
    if (!EnsureExecutablePath(hWnd, *targetPath, L"Select " + ToolDisplayName(tool) + L" Executable"))
    {
        return false;
    }

    std::wstring tempFile = GetAppDirectory() + L"\\_update_" + ToolSectionName(tool) + L".exe";
    if (!DownloadUrlOrCopy(update->DownloadUrl, tempFile))
    {
        MessageBoxW(hWnd, L"Failed to download the update package.", L"Figure Engine", MB_OK | MB_ICONERROR);
        return false;
    }

    std::wstring backup = *targetPath + L".bak";
    CopyFileW(targetPath->c_str(), backup.c_str(), FALSE);
    if (!CopyFileW(tempFile.c_str(), targetPath->c_str(), FALSE))
    {
        MessageBoxW(hWnd, L"Failed to replace the existing executable. Make sure the editor is closed.", L"Figure Engine", MB_OK | MB_ICONERROR);
        return false;
    }

    DeleteFileW(tempFile.c_str());
    SaveProjectConfig();
    SaveInstalledVersion(tool, update->AvailableVersion);
    AddLog(L"Installed update for " + ToolDisplayName(tool) + L": v" + update->AvailableVersion + L" - " + update->Title, hWnd);
    MessageBoxW(hWnd, (ToolDisplayName(tool) + L" updated to version " + update->AvailableVersion + L".\n\n" + update->Title).c_str(), L"Figure Engine", MB_OK | MB_ICONINFORMATION);
    return true;
}

void HandlePageButton(HWND hWnd, int buttonId)
{
    switch (buttonId)
    {
    case BTN_SET_LEVEL_EXE: EnsureExecutablePath(hWnd, g_Project.levelEditorExecutable, L"Select Level Editor Executable"); SaveProjectConfig(); break;
    case BTN_SET_ANIMATION_EXE: EnsureExecutablePath(hWnd, g_Project.animationEditorExecutable, L"Select Animation Editor Executable"); SaveProjectConfig(); break;
    case BTN_SET_MATERIAL_EXE: EnsureExecutablePath(hWnd, g_Project.materialEditorExecutable, L"Select Material Editor Executable"); SaveProjectConfig(); break;
    case BTN_SET_SHADER_EXE: EnsureExecutablePath(hWnd, g_Project.shaderEditorExecutable, L"Select Shader Editor Executable"); SaveProjectConfig(); break;
    case BTN_SET_UI_EXE: EnsureExecutablePath(hWnd, g_Project.uiEditorExecutable, L"Select UI Editor Executable"); SaveProjectConfig(); break;
    case BTN_SET_AUDIO_EXE: EnsureExecutablePath(hWnd, g_Project.audioEditorExecutable, L"Select Audio Editor Executable"); SaveProjectConfig(); break;
    case BTN_SET_VFX_EXE: EnsureExecutablePath(hWnd, g_Project.vfxEditorExecutable, L"Select VFX Editor Executable"); SaveProjectConfig(); break;
    case BTN_REFRESH_UPDATES: RefreshUpdates(hWnd); break;
    case BTN_DOWNLOAD_LEVEL: InstallToolUpdate(hWnd, EditorToolKind::Level); break;
    case BTN_DOWNLOAD_ANIMATION: InstallToolUpdate(hWnd, EditorToolKind::Animation); break;
    case BTN_DOWNLOAD_MATERIAL: InstallToolUpdate(hWnd, EditorToolKind::Material); break;
    case BTN_DOWNLOAD_SHADER: InstallToolUpdate(hWnd, EditorToolKind::Shader); break;
    case BTN_DOWNLOAD_UI: InstallToolUpdate(hWnd, EditorToolKind::UI); break;
    case BTN_DOWNLOAD_AUDIO: InstallToolUpdate(hWnd, EditorToolKind::Audio); break;
    case BTN_DOWNLOAD_VFX: InstallToolUpdate(hWnd, EditorToolKind::VFX); break;
    case BTN_OPEN_UPDATES_FILE:
        ShellExecuteW(hWnd, L"open", DirectoryFromPath(GetDefaultUpdatesFeedPath()).c_str(), nullptr, nullptr, SW_SHOW);
        break;
    default: break;
    }
    InvalidateRect(hWnd, nullptr, TRUE);
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
        case IDM_ABOUT: DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About); break;
        case IDM_EXIT: DestroyWindow(hWnd); break;
        case IDM_NEW_PROJECT: CreateNewProject(hWnd); break;
        case IDM_OPEN_PROJECT: OpenProject(hWnd); break;
        case IDM_SAVE_SETTINGS:
            if (EnsureCurrentProject(hWnd) && SaveProjectConfig())
            {
                AddLog(L"Saved project settings.", hWnd);
                MessageBoxW(hWnd, L"Project settings saved to the .figproj file.", L"Figure Engine", MB_OK | MB_ICONINFORMATION);
            }
            break;
        case IDM_BUILD_DEBUG: RunConfiguredCommand(hWnd, g_Project.buildDebugCommand, L"Build Debug"); SetLauncherPage(LauncherPage::Build, hWnd); break;
        case IDM_BUILD_RELEASE: RunConfiguredCommand(hWnd, g_Project.buildReleaseCommand, L"Build Release"); SetLauncherPage(LauncherPage::Build, hWnd); break;
        case IDM_PACKAGE_GAME: RunConfiguredCommand(hWnd, g_Project.packageCommand, L"Package Game"); SetLauncherPage(LauncherPage::Build, hWnd); break;
        case IDM_RUN_GAME: RunGame(hWnd, false); SetLauncherPage(LauncherPage::Run, hWnd); break;
        case IDM_RUN_DEBUG: RunGame(hWnd, true); SetLauncherPage(LauncherPage::Run, hWnd); break;
        case IDM_STOP_GAME: MessageBoxW(hWnd, L"Stop is not implemented yet. Close the launched game manually for now.", L"Figure Engine", MB_OK | MB_ICONINFORMATION); AddLog(L"Stop requested but not implemented yet.", hWnd); break;
        case IDM_TOOL_LEVEL_EDITOR: LaunchTool(hWnd, EditorToolKind::Level); SetLauncherPage(LauncherPage::Tools, hWnd); break;
        case IDM_TOOL_ANIMATION_EDITOR: LaunchTool(hWnd, EditorToolKind::Animation); SetLauncherPage(LauncherPage::Tools, hWnd); break;
        case IDM_TOOL_MATERIAL_EDITOR: LaunchTool(hWnd, EditorToolKind::Material); SetLauncherPage(LauncherPage::Tools, hWnd); break;
        case IDM_TOOL_SHADER_EDITOR: LaunchTool(hWnd, EditorToolKind::Shader); SetLauncherPage(LauncherPage::Tools, hWnd); break;
        case IDM_TOOL_UI_EDITOR: LaunchTool(hWnd, EditorToolKind::UI); SetLauncherPage(LauncherPage::Tools, hWnd); break;
        case IDM_TOOL_AUDIO_EDITOR: LaunchTool(hWnd, EditorToolKind::Audio); SetLauncherPage(LauncherPage::Tools, hWnd); break;
        case IDM_TOOL_VFX_EDITOR: LaunchTool(hWnd, EditorToolKind::VFX); SetLauncherPage(LauncherPage::Tools, hWnd); break;
        case IDM_VIEW_HOME: SetLauncherPage(LauncherPage::Home, hWnd); break;
        case IDM_VIEW_RECENT_PROJECTS: SetLauncherPage(LauncherPage::Projects, hWnd); break;
        case IDM_VIEW_PROJECT_SETTINGS: SetLauncherPage(LauncherPage::Settings, hWnd); break;
        case IDM_VIEW_ENGINE_SETTINGS: SetLauncherPage(LauncherPage::Settings, hWnd); break;
        case IDM_VIEW_BUILD_PACKAGE: SetLauncherPage(LauncherPage::Build, hWnd); break;
        case IDM_VIEW_RUN_TEST: SetLauncherPage(LauncherPage::Run, hWnd); break;
        case IDM_VIEW_TOOLS: SetLauncherPage(LauncherPage::Tools, hWnd); break;
        case IDM_VIEW_OUTPUT_LOG: SetLauncherPage(LauncherPage::OutputLog, hWnd); break;
        default: return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_LBUTTONDOWN:
    {
        RECT clientRect; GetClientRect(hWnd, &clientRect);
        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        int buttonId = HitTestButton(pt);
        if (buttonId != 0)
        {
            HandlePageButton(hWnd, buttonId);
            break;
        }
        LauncherPage clickedPage = PageFromPoint(pt, clientRect);
        if (clickedPage != g_CurrentPage) SetLauncherPage(clickedPage, hWnd);
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hWnd, &ps); DrawLauncher(hWnd, hdc); EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: return (INT_PTR)TRUE;
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
