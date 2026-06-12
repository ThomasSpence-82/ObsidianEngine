#pragma once

//
// Core DirectX 12 + DXGI headers from the Windows SDK.
//
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

//
// DX12 swapchain + RTV/DSV manager
//
// Owns:
//   - DXGI factory
//   - Swapchain
//   - RTV heap + back buffers
//   - DSV heap + depth‑stencil buffer
//
// Does NOT own:
//   - Device
//   - Command queue
//
namespace Obsidian::RHI::DX12
{
    using Microsoft::WRL::ComPtr;

    class DX12Swapchain
    {
    public:
        DX12Swapchain(
            HWND hwnd,
            UINT width,
            UINT height,
            ComPtr<ID3D12Device> device,
            ComPtr<ID3D12CommandQueue> queue);

        ~DX12Swapchain();

        // Clear current back buffer + depth buffer.
        void Clear(ComPtr<ID3D12GraphicsCommandList> cmdList, const FLOAT color[4]);

        // Present current back buffer.
        void Present(UINT sync = 1, UINT flags = 0);

        // Accessors.
        UINT GetFrameIndex() const { return m_frameIndex; }
        ComPtr<ID3D12DescriptorHeap> GetRTVHeap() const { return m_rtvHeap; }
        ComPtr<ID3D12DescriptorHeap> GetDSVHeap() const { return m_dsvHeap; }

    private:
        void CreateFactory();
        void CreateSwapchain(HWND hwnd, UINT width, UINT height);
        void CreateRTVs();
        void CreateDepthStencil(UINT width, UINT height);

    private:
        static constexpr UINT FrameCount = 2;

        // DXGI objects.
        ComPtr<IDXGIFactory7>   m_factory;
        ComPtr<IDXGISwapChain4> m_swapchain;

        // RTV heap + back buffers.
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12Resource>       m_backBuffers[FrameCount];

        // DSV heap + depth‑stencil buffer.
        ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        ComPtr<ID3D12Resource>       m_depthStencil;

        // External device + queue (not owned).
        ComPtr<ID3D12Device>        m_device;
        ComPtr<ID3D12CommandQueue>  m_queue;

        // Descriptor size for RTV heap.
        UINT m_rtvDescriptorSize = 0;

        // Current back buffer index.
        UINT m_frameIndex = 0;

        // Swapchain dimensions.
        UINT m_width = 0;
        UINT m_height = 0;
    };
}
