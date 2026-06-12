// ============================================================================
//  File: Obsidian.Runtime.cpp
//  Module: Obsidian.Runtime
//  Purpose:
//      Defines the entry point for the Win32 application. This file is
//      generated automatically by Visual Studio when creating a Win32
//      desktop application.
//
//  Why we KEEP this file (for now):
//      - It confirms the runtime project builds and runs correctly.
//      - It provides a working WinMain, window class, and message loop.
//      - It ensures the resource system (.rc, icons, menus) is functional.
//      - We will REPLACE this file later with Main.cpp, but ONLY after the
//        new entry point builds cleanly.
//
//  Notes:
//      - This file is TEMPORARY scaffolding.
//      - We do NOT modify behaviour until the engine runtime is ready.
// ============================================================================

#include "framework.h"          // Core Win32 includes and SDK targeting
#include "Obsidian.Runtime.h"   // Runtime-specific header (currently minimal)

#define MAX_LOADSTRING 100      // Maximum length for string buffers

// ============================================================================
//  Global Variables
// ============================================================================

HINSTANCE hInst;                        // Handle to the current application instance
WCHAR szTitle[MAX_LOADSTRING];          // Window title string
WCHAR szWindowClass[MAX_LOADSTRING];    // Window class name string

// ============================================================================
//  Forward Declarations
//  These functions are implemented later in this file.
// ============================================================================

ATOM                MyRegisterClass(HINSTANCE hInstance);   // Registers window class
BOOL                InitInstance(HINSTANCE, int);           // Creates and shows main window
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);    // Main window message handler
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);      // "About" dialog handler

// ============================================================================
//  FUNCTION: wWinMain
//  Purpose:
//      Application entry point for Unicode Win32 applications.
//      Initializes resources, registers window class, creates window,
//      and runs the main message loop.
// ============================================================================
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);  // Not used
    UNREFERENCED_PARAMETER(lpCmdLine);      // Not used

    // Load application title and window class name from resource file
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OBSIDIANRUNTIME, szWindowClass, MAX_LOADSTRING);

    // Register the window class
    MyRegisterClass(hInstance);

    // Create and display the main application window
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE; // Initialization failed
    }

    // Load keyboard accelerators (menu shortcuts)
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OBSIDIANRUNTIME));

    MSG msg = {};

    // ------------------------------------------------------------------------
    //  Main message loop
    //  Retrieves and dispatches messages until WM_QUIT is received.
    // ------------------------------------------------------------------------
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam; // Exit code
}

// ============================================================================
//  FUNCTION: MyRegisterClass
//  Purpose:
//      Registers the window class used to create the main application window.
// ============================================================================
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW; // Redraw on horizontal/vertical resize
    wcex.lpfnWndProc = WndProc;                 // Window procedure callback
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OBSIDIANRUNTIME));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_OBSIDIANRUNTIME);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// ============================================================================
//  FUNCTION: InitInstance
//  Purpose:
//      Saves the instance handle and creates the main application window.
// ============================================================================
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle globally

    HWND hWnd = CreateWindowW(
        szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE; // Window creation failed
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// ============================================================================
//  FUNCTION: WndProc
//  Purpose:
//      Processes messages for the main window.
// ============================================================================
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

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        // TODO: Add drawing code here...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0); // Signal application exit
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// ============================================================================
//  FUNCTION: About
//  Purpose:
//      Handles messages for the "About" dialog box.
// ============================================================================
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
