// ============================================================================
//  File: RHI_Device_DX12.h
//  Module: Obsidian.RHI.DX12
//  Purpose: Defines the DirectX 12 device interface for the Obsidian Engine.
//  Author: Thomas / Copilot
//  Created: June 2026
// ============================================================================
//  Description:
//  This header declares the RHI_Device_DX12 class, responsible for initializing,
//  managing, and shutting down the DirectX 12 graphics device and its core
//  resources (factory, adapter, device, command queue, swapchain, RTV heap).
//  The class provides a minimal bootstrap for rendering operations.
// ============================================================================

#pragma once
#include <cstdint>          // For fixed-width integer types (uint32_t)
#include <d3d12.h>          // Core Direct3D 12 definitions
#include <dxgi1_6.h>        // DXGI 1.6 for adapter and swapchain management
#include <wrl.h>            // Microsoft::WRL::ComPtr smart pointer wrapper

// ----------------------------------------------------------------------------
//  Class: RHI_Device_DX12
//  Responsibility:
//  Encapsulates DirectX 12 device creation and teardown.
//  Provides access to the DXGI factory, adapter, device, command queue,
//  swapchain, and RTV descriptor heap.
// ----------------------------------------------------------------------------
class RHI_Device_DX12
{
public:
    // ------------------------------------------------------------------------
    //  Function: Initialize
    //  Purpose:  Creates the DXGI factory, selects a hardware adapter,
    //            initializes the D3D12 device, command queue, swapchain,
    //            and RTV descriptor heap.
    //  Params:
    //      hwnd   - Handle to the window used for swapchain creation.
    //      width  - Width of the render surface.
    //      height - Height of the render surface.
    //  Returns: True if initialization succeeds, false otherwise.
    // ------------------------------------------------------------------------
    bool Initialize(HWND hwnd, uint32_t width, uint32_t height);

    // ------------------------------------------------------------------------
    //  Function: Shutdown
    //  Purpose:  Releases all DirectX 12 resources and resets COM pointers.
    // ------------------------------------------------------------------------
    void Shutdown();

private:
    // ------------------------------------------------------------------------
    //  DXGI and D3D12 core objects
    // ------------------------------------------------------------------------
    Microsoft::WRL::ComPtr<IDXGIFactory6>      m_factory;        // DXGI factory for adapter enumeration
    Microsoft::WRL::ComPtr<IDXGIAdapter1>      m_adapter;        // Selected GPU adapter
    Microsoft::WRL::ComPtr<ID3D12Device>       m_device;         // D3D12 device interface
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;   // GPU command queue
    Microsoft::WRL::ComPtr<IDXGISwapChain4>    m_swapChain;      // Swapchain for presenting frames
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;      // Render Target View descriptor heap

    // ------------------------------------------------------------------------
    //  Descriptor size cache
    // ------------------------------------------------------------------------
    UINT m_rtvDescriptorSize = 0; // Size of RTV descriptor increment
};
