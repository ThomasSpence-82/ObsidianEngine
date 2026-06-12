// ============================================================================
//  File: Main.cpp
//  Module: Obsidian.Runtime
//  Purpose:
//      Engine-oriented entry point. Creates a Win32 window, runs the main
//      message loop, and integrates the DX12 device bootstrap.
//
//  Notes:
//      - This replaces the Visual Studio template entry point.
//      - No menus, no dialogs, no accelerators.
//      - Pure engine window with minimise/maximise/close.
//      - DX12 device is created but no rendering occurs yet.
// ============================================================================

#include "framework.h"          // Win32 + SDK targeting
#include "Obsidian.Runtime.h"   // Resource IDs (icons, etc.)
#include "RHI_Device_DX12.h"    // DX12 device interface

// ----------------------------------------------------------------------------
//  Window Procedure
//  Handles basic window messages for the engine runtime.
// ----------------------------------------------------------------------------
LRESULT CALLBACK EngineWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);     // Signal application exit
        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);    // Trigger WM_DESTROY
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------
//  WinMain — Engine Entry Point
//  Creates the window, initializes DX12, and runs the main loop.
// ----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"ObsidianEngineRuntimeClass";

    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = EngineWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // Create engine window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Obsidian Engine - Runtime",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1600, 900,
        nullptr, nullptr,
        hInstance,
        nullptr);

    if (!hwnd)
        return -1;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // ------------------------------------------------------------------------
    //  DX12 Device Initialization
    // ------------------------------------------------------------------------
    RHI_Device_DX12 device;
    if (!device.Initialize(hwnd, 1600, 900))
    {
        MessageBox(hwnd, L"Failed to initialize DirectX 12 device.",
            L"DX12 Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // ------------------------------------------------------------------------
    //  Main Loop
    // ------------------------------------------------------------------------
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Future engine tick/update will go here.
        }
    }

    device.Shutdown();
    return static_cast<int>(msg.wParam);
}
