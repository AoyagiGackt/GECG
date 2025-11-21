#pragma once
#include "DirectXCommon.h"
#include <d3d12.h>
#include <string>
#include <wrl.h>

class SpriteCommon {
public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // 共通描画
    void CommonDrawSettings();

    DirectXCommon* GetDxCommon() const { return dxCommon_; }
    ID3D12Device* GetDevice() const { return dxCommon_->GetDevice(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return dxCommon_->GetCommandList(); }
    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState() const { return graphicsPipelineState_.Get(); }

private:
    DirectXCommon* dxCommon_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};