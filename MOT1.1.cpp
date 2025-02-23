#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <shlwapi.h>
#include <locale>
#include <codecvt>
#include "resource.h"
#include "MOT1.1.h"
#include "winres.h"

// Link necessary libraries
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

namespace fs = std::filesystem;

// Define constants for control IDs
#define ID_REFRESH 1
#define ID_CREATE 2
#define ID_LISTVIEW 1001
#define ID_NEXUS_LINK 1000

// Global variables for UI elements
HFONT hFont;
HWND hListView;
HWND hLabelInfo;
HWND hBtnRefresh;
HWND hBtnCreate;
HWND hNexus;

std::vector<std::wstring> modFolders;

// --------------------
// Font and Drawing Functions
// --------------------

// Function to create a Segoe UI font
HFONT CreateSegoeUIFont()
{
    return CreateFont(
        18, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
}

// Function to draw a background with rounded corners
void DrawRoundedBackground(HDC hdc, RECT rect)
{
    // Background color
    HBRUSH hBrush = CreateSolidBrush(RGB(218, 142, 53)); // Color #da8e35
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);

    // Draw rounded corners
    int cornerRadius = 10; // Radius for rounded corners
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, cornerRadius, cornerRadius);
}

// --------------------
// Mod Folder Functions
// --------------------

// Function to load mod folders from the current directory
void LoadModFolders(HWND hwnd)
{
    modFolders.clear();
    ListView_DeleteAllItems(hListView);

    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    PathRemoveFileSpec(path);

    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            std::wstring folderName = entry.path().filename().wstring();
            modFolders.push_back(folderName);

            wchar_t folderNameBuffer[MAX_PATH];
            wcscpy_s(folderNameBuffer, MAX_PATH, folderName.c_str());

            LVITEM lvItem = {};
            lvItem.mask = LVIF_TEXT;
            lvItem.pszText = folderNameBuffer;
            lvItem.iItem = ListView_GetItemCount(hListView);
            ListView_InsertItem(hListView, &lvItem);
        }
    }

    bool renamed = false;

    for (auto& folder : modFolders)
    {
        if (folder.find(L' ') != std::wstring::npos || folder.find(L'_') != std::wstring::npos)
        {
            if (!renamed)
            {
                int result = MessageBox(hwnd, L"Some folder names contain white-spaces. Rename them?", L"Rename Folders", MB_YESNO | MB_ICONQUESTION);
                if (result == IDYES)
                {
                    renamed = true; // Set flag to indicate folders should be renamed
                }
            }

            if (renamed)
            {
                std::wstring newName = folder;
                newName.erase(std::remove(newName.begin(), newName.end(), L' '), newName.end()); // Remove spaces
                newName.erase(std::remove(newName.begin(), newName.end(), L'_'), newName.end()); // Remove underscores

                std::wstring oldPath = path + std::wstring(L"\\") + folder;
                std::wstring newPath = path + std::wstring(L"\\") + newName;

                try
                {
                    // Check if the folder exists before attempting to rename
                    if (fs::exists(oldPath))
                    {
                        fs::rename(oldPath, newPath); // Rename the folder
                        folder = newName;             // Update the name in the vector
                    }
                    else
                    {
                        MessageBox(hwnd, L"Folder not found. Cannot rename.", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
                catch (const std::exception& e)
                {
                    wchar_t buffer[1024];
                    mbstowcs_s(nullptr, buffer, e.what(), 1024);
                    std::wstring message = L"Error renaming folder: " + std::wstring(buffer);
                    MessageBox(hwnd, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
                }
            }
        }
    }

    wchar_t infoText[100];
    swprintf_s(infoText, L"Found %llu mods", modFolders.size());
    SetWindowText(hLabelInfo, infoText);
}

// Function to create a mod order file
void CreateModOrder(HWND hwnd)
{
    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    PathRemoveFileSpec(path);

    std::wstring modOrderPath = path + std::wstring(L"\\mod_order.txt");

    FILE* file;
    _wfopen_s(&file, modOrderPath.c_str(), L"w, ccs=UTF-8");

    if (file)
    {
        for (const auto& folder : modFolders)
        {
            fwprintf(file, L"%s\n", folder.c_str());
        }
        fclose(file);
        MessageBox(hwnd, L"mod_order.txt created successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        MessageBox(hwnd, L"Failed to create mod_order.txt.", L"Error", MB_OK | MB_ICONERROR);
    }
}

// --------------------
// Window Procedure
// --------------------

// Window procedure for handling messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg)
    {
    case WM_CREATE:
    {
        HFONT hFont = CreateSegoeUIFont();

        // Set fonts for various controls
        SendMessage(hLabelInfo, WM_SETFONT, WPARAM(hFont), TRUE);
        SendMessage(hBtnRefresh, WM_SETFONT, WPARAM(hFont), TRUE);
        SendMessage(hBtnCreate, WM_SETFONT, WPARAM(hFont), TRUE);
        SendMessage(hNexus, WM_SETFONT, WPARAM(hFont), TRUE);

        hLabelInfo = CreateWindowW(L"STATIC", L"Found 0 mods", WS_CHILD | WS_VISIBLE | SS_CENTER,
            0, 10, 410, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hLabelInfo, WM_SETFONT, WPARAM(hFont), TRUE);

        hListView = CreateWindowW(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
            0, 40, 400, 300, hwnd, (HMENU)ID_LISTVIEW, NULL, NULL);

        LVCOLUMN lvColumn = {};
        lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
        lvColumn.cx = 250;
        ListView_InsertColumn(hListView, 0, &lvColumn);
        SendMessage(hListView, WM_SETFONT, WPARAM(hFont), TRUE);

        hBtnRefresh = CreateWindowW(L"BUTTON", L"Refresh mods", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            20, 350, 140, 40, hwnd, (HMENU)ID_REFRESH, NULL, NULL);
        SendMessage(hBtnRefresh, WM_SETFONT, WPARAM(hFont), TRUE);

        hBtnCreate = CreateWindowW(L"BUTTON", L"Create mod_order.txt", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            212, 350, 180, 40, hwnd, (HMENU)ID_CREATE, NULL, NULL);
        SendMessage(hBtnCreate, WM_SETFONT, WPARAM(hFont), TRUE);

        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        int windowWidth = rcClient.right;
        // Setting identifier for hNexus
        hNexus = CreateWindowW(L"STATIC", L"                                by itsbttq for nexusmods",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOTIFY,
            0, 440, windowWidth - 0, 50, hwnd, (HMENU)ID_NEXUS_LINK, NULL, NULL);
        SendMessage(hNexus, WM_SETFONT, WPARAM(hFont), TRUE);

        LoadModFolders(hwnd);
    }
    break;

    case WM_CTLCOLORBTN:
    {
        HDC hdcButton = (HDC)wParam;
        HWND hwndButton = (HWND)lParam;

        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;

        // Check if the control is the one we want to change the background for
        if (hwndStatic == hNexus)
        {
            HBRUSH hBrush = CreateSolidBrush(RGB(218, 142, 53));
            SetBkColor(hdcStatic, RGB(218, 142, 53));
            SetTextColor(hdcStatic, RGB(255, 255, 255));

            return (LRESULT)hBrush; // Return the brush handle
        }
        if (hwndStatic == hLabelInfo)
        {
            HBRUSH hBrush = CreateSolidBrush(RGB(68, 68, 68));
            SetBkColor(hdcStatic, RGB(68, 68, 68));
            SetTextColor(hdcStatic, RGB(129, 213, 97));

            return (LRESULT)hBrush; // Return the brush handle
        }

        return DefWindowProc(hwnd, msg, wParam, lParam); // Default processing for other controls
    }
    break;

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

        if (lpdis->CtlID == ID_REFRESH || lpdis->CtlID == ID_CREATE)
        {
            // Buttons have custom background and text colors
            HDC hdc = lpdis->hDC;
            RECT rc = lpdis->rcItem;

            // Change background and text color
            if (lpdis->CtlID == ID_REFRESH)
            {
                SetBkColor(hdc, RGB(97, 97, 97));
                SetTextColor(hdc, RGB(255, 255, 255));
            }
            else if (lpdis->CtlID == ID_CREATE)
            {
                SetBkColor(hdc, RGB(97, 97, 97));
                SetTextColor(hdc, RGB(255, 255, 255));
            }

            // Draw the background
            FillRect(hdc, &rc, CreateSolidBrush(GetBkColor(hdc)));

            // Read the button text
            TCHAR buttonText[256];
            GetWindowText(lpdis->hwndItem, buttonText, sizeof(buttonText) / sizeof(TCHAR));

            // Draw the text
            DrawText(hdc, buttonText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Ensure default drawing
            return TRUE; // Stop further processing
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_NEXUS_LINK)
        {
            ShellExecute(NULL, L"open", L"https://www.nexusmods.com/kingdomcomedeliverance/mods/1829", NULL, NULL, SW_SHOWNORMAL);
        }
        else
        {
            // Handle button clicks
            switch (LOWORD(wParam))
            {
            case ID_REFRESH:
                LoadModFolders(hwnd);
                break;
            case ID_CREATE:
                CreateModOrder(hwnd);
                break;
            }
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        DeleteObject(hFont);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// --------------------
// Application Initialization
// --------------------

// Function to initialize the application
void InitApp(HINSTANCE hInstance)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"KCDModManager";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hbrBackground = CreateSolidBrush(RGB(68, 68, 68));

    RegisterClass(&wc);
}

// --------------------
// Main Entry Point
// --------------------

// Entry point for the application
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    InitApp(hInstance);

    HWND hwnd = CreateWindowEx(0, L"KCDModManager", L"KCD: Mod Order Tool",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 430, 500, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        return 0;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hInstance);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
