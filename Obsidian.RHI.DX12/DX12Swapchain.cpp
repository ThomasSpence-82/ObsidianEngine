#include "pch.h"                 // Precompiled header (VS setting)
#include "DX12Swapchain.h"

#include <stdexcept>             // std::runtime_error
#include <windows.h>             // HWND, Win32 types
#include <d3d12.h>               // Core D3D12 types
#include <dxgi1_6.h>             // DXGI interfaces

// Simple HRESULT check macro.
// In a production engine you’d route this through your logging/assert system.
#ifndef DX_CALL
#define DX_CALL(x) do { HRESULT _hr = (x); if (FAILED(_hr)) throw std::runtime_error("DX12 call failed: " #x); } while(0)
#endif

using namespace Obsidian::RHI::DX12;
using Microsoft::WRL::ComPtr;

// -------------------------------------------------------------------------------------------------
// Constructor: stores device/queue, creates factory, swapchain, and RTVs.
// -------------------------------------------------------------------------------------------------
DX12Swapchain::DX12Swapchain(
    HWND hwnd,
    UINT width,
    UINT height,
    ComPtr<ID3D12Device> device,
    ComPtr<ID3D12CommandQueue> queue)
    : m_device(device)
    , m_queue(queue)
    , m_width(width)
    , m_height(height)
{
    CreateFactory();
    CreateSwapchain(hwnd, width, height);
    CreateRTVs();
}

// -------------------------------------------------------------------------------------------------
// Destructor: release back buffers, heap, swapchain, factory.
// Device/queue are external and not owned here.
// -------------------------------------------------------------------------------------------------
DX12Swapchain::~DX12Swapchain()
{
    for (auto& bb : m_backBuffers)
        bb.Reset();

    m_rtvHeap.Reset();
    m_swapchain.Reset();
    m_factory.Reset();
}

// -------------------------------------------------------------------------------------------------
// Create DXGI factory (debug layer enabled in debug builds).
// -------------------------------------------------------------------------------------------------
void DX12Swapchain::CreateFactory()
{
    UINT flags = 0;
#if _DEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    DX_CALL(CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_factory)));
}

// -------------------------------------------------------------------------------------------------
// Create flip-discard swapchain for the given HWND.
// -------------------------------------------------------------------------------------------------
void DX12Swapchain::CreateSwapchain(HWND hwnd, UINT width, UINT height)
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.BufferCount = FrameCount;
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    ComPtr<IDXGISwapChain1> temp;

    DX_CALL(m_factory->CreateSwapChainForHwnd(
        m_queue.Get(),   // command queue
        hwnd,            // target window
        &desc,           // swapchain description
        nullptr,         // fullscreen desc (unused)
        nullptr,         // restrict output (unused)
        &temp));

    DX_CALL(temp.As(&m_swapchain));

    // Cache current back buffer index.
    m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
}

// -------------------------------------------------------------------------------------------------
// Create RTV descriptor heap and RTVs for each back buffer.
// -------------------------------------------------------------------------------------------------
void DX12Swapchain::CreateRTVs()
{
    // Describe and create the RTV descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = FrameCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    DX_CALL(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    // Cache descriptor size for RTV heap.
    m_rtvDescriptorSize =
        m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Get the starting CPU handle for the heap.
    D3D12_CPU_DESCRIPTOR_HANDLE handle =
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Create an RTV for each back buffer.
    for (UINT i = 0; i < FrameCount; i++)
    {
        DX_CALL(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])));
        m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);

        // Move handle to the next descriptor slot.
        handle.ptr += m_rtvDescriptorSize;
    }
}

// -------------------------------------------------------------------------------------------------
// Clear: transition PRESENT → RENDER_TARGET, clear, then RENDER_TARGET → PRESENT.
// This keeps Present() happy and avoids invalid resource state crashes.
// -------------------------------------------------------------------------------------------------
void DX12Swapchain::Clear(ComPtr<ID3D12GraphicsCommandList> cmdList, const FLOAT color[4])
{
    // Transition from PRESENT to RENDER_TARGET.
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_backBuffers[m_frameIndex].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        cmdList->ResourceBarrier(1, &barrier);
    }

    // Compute RTV handle for current back buffer.
    D3D12_CPU_DESCRIPTOR_HANDLE rtv =
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += static_cast<SIZE_T>(m_frameIndex) * m_rtvDescriptorSize;

    // Bind and clear.
    cmdList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    cmdList->ClearRenderTargetView(rtv, color, 0, nullptr);

    // Transition back to PRESENT so the swapchain can present safely.
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_backBuffers[m_frameIndex].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        cmdList->ResourceBarrier(1, &barrier);
    }
}

// -------------------------------------------------------------------------------------------------
// Present: call DXGI Present and update frame index.
// Any failure here will throw via DX_CALL.
// -------------------------------------------------------------------------------------------------
void DX12Swapchain::Present(UINT sync, UINT flags)
{
    DX_CALL(m_swapchain->Present(sync, flags));
    m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
}
