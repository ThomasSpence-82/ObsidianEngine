// Obsidian.RHI.DX12.cpp : Defines the functions for the static library.
//
// ============================================================================
//  File:    Obsidian.RHI.DX12.cpp
//  Module:  Obsidian.RHI.DX12
//  Purpose: Implements the DirectX 12 backend for the Obsidian Engine.
//  Author:  Thomas / Copilot
//  Created: June 2026
// ============================================================================
//  Description:
//  This file contains the implementation of the RHI_Device_DX12 class,
//  responsible for initializing and shutting down the DirectX 12 device,
//  command queue, swapchain, and RTV descriptor heap.
//
//  NOTE:
//  - We KEEP the default Visual Studio includes (pch.h, framework.h).
//  - We extend this file instead of replacing it.
//  - This ensures stable build settings and avoids breaking the PCH system.
// ============================================================================

#include "pch.h"                  // Precompiled header (must be first include)
#include "framework.h"            // Windows platform definitions
#include "RHI_Device_DX12.h"      // Our DX12 device class declaration

// Core DX12 + DXGI + WRL headers from the Windows SDK.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace Obsidian::RHI::DX12
{

    // ============================================================================
    //  Default Visual Studio placeholder function
    //  (We keep this until the project is fully structured.)
    // ============================================================================
    void fnObsidianRHIDX12()
    {
        // Intentionally left empty.
        // This function exists only because Visual Studio generates it by default.
        // We will remove it once the RHI module is fully populated.
    }

    // ============================================================================
    //  Function: RHI_Device_DX12::Initialize
    //  Purpose:
    //      Bootstraps the DirectX 12 rendering backend by creating:
    //        - DXGI factory
    //        - Hardware adapter
    //        - D3D12 device
    //        - Command queue
    //        - Swapchain
    //        - RTV descriptor heap
    //
    //  This is the foundation of all GPU rendering in the engine.
    // ============================================================================
    bool RHI_Device_DX12::Initialize(HWND hwnd, uint32_t width, uint32_t height)
    {
        // ------------------------------------------------------------------------
        // 0. Enable D3D12 debug layer in debug builds (helps catch GPU errors early)
        // ------------------------------------------------------------------------
#if defined(_DEBUG)
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
            }
        }
#endif

    // ------------------------------------------------------------------------
    // 1. Create DXGI factory (responsible for adapter enumeration + swapchain)
    // ------------------------------------------------------------------------
    UINT dxgiFlags = 0;
#if defined(_DEBUG)
    dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    if (FAILED(CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&m_factory))))
    {
        return false;
    }

    // ------------------------------------------------------------------------
    // 2. Select the best available hardware adapter
    //    - Skip software adapters (Microsoft Basic Render Driver)
    //    - Prefer the first hardware adapter found
    // ------------------------------------------------------------------------
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0;
        m_factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND;
        ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc = {};
        adapter->GetDesc1(&desc);

        // Skip software adapters.
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;

        // Found a hardware adapter; store it and break.
        m_adapter = adapter;
        break;
    }

    if (!m_adapter)
    {
        // No suitable hardware adapter found.
        return false;
    }

    // ------------------------------------------------------------------------
    // 3. Create the Direct3D 12 device
    // ------------------------------------------------------------------------
    if (FAILED(D3D12CreateDevice(
        m_adapter.Get(),
        D3D_FEATURE_LEVEL_12_1,
        IID_PPV_ARGS(&m_device))))
    {
        return false;
    }

    // ------------------------------------------------------------------------
    // 4. Create the GPU command queue
    //    (All rendering commands are submitted through this queue)
    // ------------------------------------------------------------------------
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue))))
    {
        return false;
    }

    // ------------------------------------------------------------------------
    // 5. Create the swapchain (manages backbuffers for presentation)
    // ------------------------------------------------------------------------
    DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
    swapDesc.BufferCount = 2; // Double buffering
    swapDesc.Width = width;
    swapDesc.Height = height;
    swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;

    ComPtr<IDXGISwapChain1> tempSwapChain;
    if (FAILED(m_factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),   // queue that will present
        hwnd,                   // target window
        &swapDesc,              // swapchain description
        nullptr,                // fullscreen desc (unused)
        nullptr,                // restrict output (unused)
        &tempSwapChain)))
    {
        return false;
    }

    // Upgrade to IDXGISwapChain4 for modern features.
    if (FAILED(tempSwapChain.As(&m_swapChain)))
    {
        return false;
    }

    // ------------------------------------------------------------------------
    // 6. Create RTV descriptor heap (stores render target views)
    // ------------------------------------------------------------------------
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.NumDescriptors = 2; // One per backbuffer
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(m_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_rtvHeap))))
    {
        return false;
    }

    // Cache descriptor size for later use (RTV handle increments).
    m_rtvDescriptorSize =
        m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    return true;
    }

    // ============================================================================
    //  Function: RHI_Device_DX12::Shutdown
    //  Purpose:
    //      Releases all DirectX 12 resources in a safe order.
    // ============================================================================
    void RHI_Device_DX12::Shutdown()
    {
        // Release in reverse order of creation where practical.
        m_rtvHeap.Reset();
        m_swapChain.Reset();
        m_commandQueue.Reset();
        m_device.Reset();
        m_adapter.Reset();
        m_factory.Reset();
    }

} // namespace Obsidian::RHI::DX12
