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
//
//  This class provides the minimal bootstrap required for the renderer to
//  submit GPU work and present frames to the screen.
// ============================================================================

#pragma once

// ---------------------------------------------------------------------------
// Standard + Windows SDK includes
// ---------------------------------------------------------------------------
#include <cstdint>          // uint32_t
#include <windows.h>        // HWND, Win32 types
#include <d3d12.h>          // Core Direct3D 12 API
#include <dxgi1_6.h>        // DXGI 1.6 for adapter + swapchain
#include <wrl.h>            // Microsoft::WRL::ComPtr smart pointer

// ---------------------------------------------------------------------------
// Namespace: Obsidian::RHI::DX12
// The DX12 backend lives inside the engine's RHI namespace.
// ---------------------------------------------------------------------------
namespace Obsidian::RHI::DX12
{
    // -----------------------------------------------------------------------
    //  Class: RHI_Device_DX12
    //
    //  Responsibility:
    //      Encapsulates DirectX 12 device creation and teardown.
    //      Provides access to:
    //          - DXGI factory
    //          - Hardware adapter
    //          - D3D12 device
    //          - Command queue
    //          - Swapchain
    //          - RTV descriptor heap
    //
    //  This is the foundation of all GPU rendering in the engine.
    // -----------------------------------------------------------------------
    class RHI_Device_DX12
    {
    public:

        // -------------------------------------------------------------------
        // Initialize()
        //
        // Creates:
        //   - DXGI factory
        //   - Hardware adapter
        //   - D3D12 device
        //   - Command queue
        //   - Swapchain
        //   - RTV descriptor heap
        //
        // Returns:
        //   true  = success
        //   false = failure (device not supported, no adapter, etc.)
        // -------------------------------------------------------------------
        bool Initialize(HWND hwnd, uint32_t width, uint32_t height);

        // -------------------------------------------------------------------
        // Shutdown()
        //
        // Releases all DirectX 12 resources in a safe order.
        // -------------------------------------------------------------------
        void Shutdown();

    private:

        // -------------------------------------------------------------------
        // DXGI + D3D12 core objects
        //
        // These are the essential building blocks of a DX12 renderer.
        // -------------------------------------------------------------------
        Microsoft::WRL::ComPtr<IDXGIFactory6>        m_factory;        // DXGI factory (adapter enumeration + swapchain)
        Microsoft::WRL::ComPtr<IDXGIAdapter1>        m_adapter;        // Selected hardware GPU adapter
        Microsoft::WRL::ComPtr<ID3D12Device>         m_device;         // D3D12 device (root of all GPU resources)
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>   m_commandQueue;   // GPU command queue (submits work)
        Microsoft::WRL::ComPtr<IDXGISwapChain4>      m_swapChain;      // Swapchain (manages backbuffers)
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;        // RTV descriptor heap (stores render target views)

        // -------------------------------------------------------------------
        // Cached descriptor size
        //
        // RTV descriptors are stored in a contiguous heap. The device reports
        // the increment size so we can compute CPU descriptor handles.
        // -------------------------------------------------------------------
        UINT m_rtvDescriptorSize = 0;
    };
} // namespace Obsidian::RHI::DX12
