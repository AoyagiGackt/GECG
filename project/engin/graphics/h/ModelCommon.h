#pragma once
#include "DirectXCommon.h"
#include <wrl/client.h>

class ModelCommon {
public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon);
    // 描画共通設定
    void CommonDrawSettings();

    DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
    DirectXCommon* dxCommon_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};