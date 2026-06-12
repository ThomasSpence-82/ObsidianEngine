#include "pch.h"
#include "DX12Swapchain.h"

#include <stdexcept>
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#ifndef DX_CALL
#define DX_CALL(x) do { HRESULT _hr = (x); if (FAILED(_hr)) throw std::runtime_error("DX12 call failed: " #x); } while(0)
#endif

using namespace Obsidian::RHI::DX12;
using Microsoft::WRL::ComPtr;

// -------------------------------------------------------------------------------------------------
// Constructor: stores device/queue, creates factory, swapchain, RTVs, and depth‑stencil.
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
    CreateDepthStencil(width, height);
}

// -------------------------------------------------------------------------------------------------
// Destructor: release back buffers, depth buffer, heaps, swapchain, factory.
// -------------------------------------------------------------------------------------------------
DX12Swapchain::~DX12Swapchain()
{
    for (auto& bb : m_backBuffers)
        bb.Reset();

    m_depthStencil.Reset();
    m_dsvHeap.Reset();
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
        m_queue.Get(),
        hwnd,
        &desc,
        nullptr,
        nullptr,
        &temp));

    DX_CALL(temp.As(&m_swapchain));

    m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
}

// -------------------------------------------------------------------------------------------------
// Create RTV descriptor heap and RTVs for each back buffer.
// -------------------------------------------------------------------------------------------------
void DX12Swapchain::CreateRTVs()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = FrameCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    DX_CALL(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    m_rtvDescriptorSize =
        m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE handle =
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < FrameCount; i++)
    {
        DX_CALL(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])));
        m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);
        handle.ptr += m_rtvDescriptorSize;
    }
}

// -------------------------------------------------------------------------------------------------
// Create depth‑stencil buffer + DSV heap.
// -------------------------------------------------------------------------------------------------
void DX12Swapchain::CreateDepthStencil(UINT width, UINT height)
{
    // Describe the depth‑stencil texture.
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // Clear value for depth.
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    // Heap properties for a default GPU resource.
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    // Create the depth‑stencil resource.
    DX_CALL(m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&m_depthStencil)));

    // Create DSV heap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
    dsvDesc.NumDescriptors = 1;
    dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    DX_CALL(m_device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&m_dsvHeap)));

    // Create the DSV.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvView = {};
    dsvView.Format = DXGI_FORMAT_D32_FLOAT;
    dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvView.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvView, dsvHandle);
}

// -------------------------------------------------------------------------------------------------
// Clear: transition PRESENT → RENDER_TARGET, clear color + depth, then back.
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

    // RTV handle for current back buffer.
    D3D12_CPU_DESCRIPTOR_HANDLE rtv =
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += static_cast<SIZE_T>(m_frameIndex) * m_rtvDescriptorSize;

    // DSV handle (single depth buffer).
    D3D12_CPU_DESCRIPTOR_HANDLE dsv =
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    // Bind RTV + DSV.
    cmdList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    // Clear color + depth.
    cmdList->ClearRenderTargetView(rtv, color, 0, nullptr);
    cmdList->ClearDepthStencilView(
        dsv,
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f,
        0,
        0,
        nullptr);

    // Transition back to PRESENT.
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
// -------------------------------------------------------------------------------------------------
void DX12Swapchain::Present(UINT sync, UINT flags)
{
    DX_CALL(m_swapchain->Present(sync, flags));
    m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
}
