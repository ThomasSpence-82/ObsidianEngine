#include "pch.h"                      // Precompiled header

// Core Win32 + DX12 headers from the Windows SDK.
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <memory>                    // std::unique_ptr, std::make_unique

#include "DX12Swapchain.h"           // From Obsidian.RHI.DX12 (project reference)

#ifndef DX_CALL
#define DX_CALL(x) do { HRESULT _hr = (x); if (FAILED(_hr)) { OutputDebugStringA("DX12 call failed: " #x "\n"); ::ExitProcess(static_cast<UINT>(_hr)); } } while(0)
#endif

using Microsoft::WRL::ComPtr;
using Obsidian::RHI::DX12::DX12Swapchain;

// -------------------------------------------------------------------------------------------------
// Global DX12 objects.
// -------------------------------------------------------------------------------------------------
static ComPtr<ID3D12Device>              g_device;
static ComPtr<ID3D12CommandQueue>        g_queue;
static ComPtr<ID3D12CommandAllocator>    g_cmdAlloc;
static ComPtr<ID3D12GraphicsCommandList> g_cmdList;
static std::unique_ptr<DX12Swapchain>    g_swapchain;

static ComPtr<ID3D12Fence> g_fence;
static UINT64              g_fenceValue = 0;
static HANDLE              g_fenceEvent = nullptr;

// Cache window size for viewport/scissor.
static UINT g_backbufferWidth = 0;
static UINT g_backbufferHeight = 0;

// -------------------------------------------------------------------------------------------------
// Basic Win32 window procedure.
// -------------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

// -------------------------------------------------------------------------------------------------
// Create a simple Win32 window.
// -------------------------------------------------------------------------------------------------
HWND CreateMainWindow(HINSTANCE hInstance, int width, int height)
{
    const wchar_t* className = L"ObsidianRuntimeWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(
        0,
        className,
        L"Obsidian Engine",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}

// -------------------------------------------------------------------------------------------------
// Initialize D3D12 device, queue, command allocator/list, fence, and swapchain.
// -------------------------------------------------------------------------------------------------
void InitD3D(HWND hwnd, UINT width, UINT height)
{
#if _DEBUG
    {
        ComPtr<ID3D12Debug> debug;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
            debug->EnableDebugLayer();
    }
#endif

    g_backbufferWidth = width;
    g_backbufferHeight = height;

    DX_CALL(D3D12CreateDevice(
        nullptr,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&g_device)));

    D3D12_COMMAND_QUEUE_DESC qd = {};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    DX_CALL(g_device->CreateCommandQueue(&qd, IID_PPV_ARGS(&g_queue)));

    DX_CALL(g_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&g_cmdAlloc)));

    DX_CALL(g_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        g_cmdAlloc.Get(),
        nullptr,
        IID_PPV_ARGS(&g_cmdList)));

    DX_CALL(g_cmdList->Close());

    DX_CALL(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));
    g_fenceValue = 0;
    g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    g_swapchain = std::make_unique<DX12Swapchain>(
        hwnd,
        width,
        height,
        g_device,
        g_queue);
}

// -------------------------------------------------------------------------------------------------
// Wait for GPU to reach the current fence value.
// -------------------------------------------------------------------------------------------------
void WaitForGPU()
{
    g_fenceValue++;
    DX_CALL(g_queue->Signal(g_fence.Get(), g_fenceValue));

    if (g_fence->GetCompletedValue() < g_fenceValue)
    {
        DX_CALL(g_fence->SetEventOnCompletion(g_fenceValue, g_fenceEvent));
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }
}

// -------------------------------------------------------------------------------------------------
// Render a single frame: viewport, scissor, reset, clear, execute, wait, present.
// -------------------------------------------------------------------------------------------------
void RenderFrame()
{
    DX_CALL(g_cmdAlloc->Reset());
    DX_CALL(g_cmdList->Reset(g_cmdAlloc.Get(), nullptr));

    // Viewport.
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(g_backbufferWidth);
    viewport.Height = static_cast<float>(g_backbufferHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    g_cmdList->RSSetViewports(1, &viewport);

    // Scissor rect.
    D3D12_RECT scissor = {};
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = static_cast<LONG>(g_backbufferWidth);
    scissor.bottom = static_cast<LONG>(g_backbufferHeight);
    g_cmdList->RSSetScissorRects(1, &scissor);

    // Clear current back buffer (handles PRESENT ↔ RENDER_TARGET transitions).
    const FLOAT clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f };
    g_swapchain->Clear(g_cmdList, clearColor);

    DX_CALL(g_cmdList->Close());

    ID3D12CommandList* lists[] = { g_cmdList.Get() };
    g_queue->ExecuteCommandLists(1, lists);

    WaitForGPU();
    g_swapchain->Present(1, 0);
}

// -------------------------------------------------------------------------------------------------
// Clean up GPU resources.
// -------------------------------------------------------------------------------------------------
void ShutdownD3D()
{
    WaitForGPU();

    g_swapchain.reset();
    g_cmdList.Reset();
    g_cmdAlloc.Reset();
    g_queue.Reset();
    g_device.Reset();

    if (g_fenceEvent)
    {
        CloseHandle(g_fenceEvent);
        g_fenceEvent = nullptr;
    }

    g_fence.Reset();
}

// -------------------------------------------------------------------------------------------------
// Standard Win32 entry point.
// -------------------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    const UINT width = 1280;
    const UINT height = 720;

    HWND hwnd = CreateMainWindow(hInstance, width, height);
    InitD3D(hwnd, width, height);

    MSG msg = {};
    bool running = true;

    while (running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running)
            break;

        RenderFrame();
    }

    ShutdownD3D();
    return 0;
}
