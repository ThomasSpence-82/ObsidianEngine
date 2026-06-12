#pragma once

//
// Core DirectX 12 + DXGI headers from the Windows SDK.
// These are the ONLY headers you need for swapchain + RTV management.
// No DirectX-Headers package, no d3dx12.h helpers.
//
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

//
// DX12 swapchain + RTV manager
//
// This class owns:
//   - DXGI factory (IDXGIFactory7)
//   - Swapchain (IDXGISwapChain4)
//   - RTV descriptor heap
//   - Back buffer resources
//
// It does NOT own:
//   - The D3D12 device
//   - The command queue
//
// The device + queue are passed in from the renderer and remain externally owned.
// This keeps the swapchain lightweight and modular.
//
namespace Obsidian::RHI::DX12
{
    using Microsoft::WRL::ComPtr;

    class DX12Swapchain
    {
    public:

        //
        // Constructor
        // Creates:
        //   - DXGI factory
        //   - Swapchain for the given HWND
        //   - RTV heap + RTVs for each back buffer
        //
        DX12Swapchain(
            HWND hwnd,
            UINT width,
            UINT height,
            ComPtr<ID3D12Device> device,
            ComPtr<ID3D12CommandQueue> queue);

        //
        // Destructor
        // Releases:
        //   - Back buffers
        //   - RTV heap
        //   - Swapchain
        //   - Factory
        //
        ~DX12Swapchain();

        //
        // Clear the current back buffer.
        // Performs:
        //   - PRESENT → RENDER_TARGET transition
        //   - ClearRenderTargetView()
        //   - RENDER_TARGET → PRESENT transition
        //
        void Clear(ComPtr<ID3D12GraphicsCommandList> cmdList, const FLOAT color[4]);

        //
        // Present the current back buffer.
        // Updates m_frameIndex after presenting.
        //
        void Present(UINT sync = 1, UINT flags = 0);

        //
        // Accessors
        //
        UINT GetFrameIndex() const { return m_frameIndex; }
        ComPtr<ID3D12DescriptorHeap> GetRTVHeap() const { return m_rtvHeap; }

    private:

        //
        // Internal creation helpers
        //
        void CreateFactory();
        void CreateSwapchain(HWND hwnd, UINT width, UINT height);
        void CreateRTVs();

    private:

        //
        // Number of back buffers.
        // 2 = double buffering (recommended for most engines).
        //
        static constexpr UINT FrameCount = 2;

        //
        // DXGI objects
        //
        ComPtr<IDXGIFactory7>     m_factory;
        ComPtr<IDXGISwapChain4>   m_swapchain;

        //
        // RTV heap + back buffer resources
        //
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12Resource>       m_backBuffers[FrameCount];

        //
        // External device + queue (not owned)
        //
        ComPtr<ID3D12Device>        m_device;
        ComPtr<ID3D12CommandQueue>  m_queue;

        //
        // Cached descriptor size for RTV heap
        //
        UINT m_rtvDescriptorSize = 0;

        //
        // Current back buffer index
        //
        UINT m_frameIndex = 0;

        //
        // Swapchain dimensions
        //
        UINT m_width = 0;
        UINT m_height = 0;
    };
}
