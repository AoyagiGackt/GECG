#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <wrl.h>

class WinApp;

// DirectX基盤
class DirectXCommon {
public: // メンバ関数
    // 初期化
    void Initialize();

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
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_;
    HANDLE fenceEvent_;
};