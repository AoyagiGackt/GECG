#pragma once
#include <cassert>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class WinApp;

// DirectX基盤
class DirectXCommon {
public: // メンバ関数
    // 初期化
    void Initialize(WinApp* winApp);

    ID3D12Device* GetDevice() { return device_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); }
    ID3D12CommandQueue* GetCommandQueue() { return commandQueue_.Get(); }
    IDXGISwapChain4* GetSwapChain() { return swapChain_.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() { return commandAllocator_.Get(); }
    ID3D12DescriptorHeap* GetSrvDescriptorHeap() { return srvDescriptorHeap_.Get(); }

    // RTV関連
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferHandle();
    ID3D12Resource* GetCurrentBackBufferResource();
    DXGI_FORMAT GetBackBufferFormat() const { return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; }
    UINT GetBufferCount() const { return 2; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() { return dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(); }

    // フェンス関連
    ID3D12Fence* GetFence() { return fence_.Get(); }
    uint64_t GetFenceValue() const { return fenceValue_; }
    HANDLE GetFenceEvent() { return fenceEvent_; }
    void IncrementFenceValue() { fenceValue_++; }

private:
    void InitializeDevice();
    void CreateCommand();
    void CreateSwapChain();
    void CreateDescriptorHeaps();
    void CreateRTV();
    void CreateDepthBuffer();
    void CreateFence();

private:
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResoures_[2];
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_;
    HANDLE fenceEvent_;

    WinApp* winApp_ = nullptr;
};