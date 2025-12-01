#pragma once
#include "DirectXCommon.h"
#include <wrl/client.h>

class SrvManager {
public:
    // シングルトン
    static SrvManager* GetInstance();

    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // 描画前準備
    void PreDraw();

    // SRV用デスクリプタの確保
    uint32_t Allocate();

    // テクスチャ2D用のSRV生成
    void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);

    // 終了処理
    void Finalize();

    ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return descriptorHeap_.Get(); }

    // CPUハンドルを取得
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);

    // GPUハンドルを取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

private:
    SrvManager() = default;
    ~SrvManager() = default;
    SrvManager(const SrvManager&) = delete;
    SrvManager& operator=(const SrvManager&) = delete;

    DirectXCommon* dxCommon_ = nullptr;

    // SRV用デスクリプタヒープ
    static const uint32_t kMaxSRVCount = 128; // 最大数
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

    // デスクリプタ1個分のサイズ
    uint32_t descriptorSize_;

    // 次に使用するインデックス
    uint32_t useIndex_ = 0;
};