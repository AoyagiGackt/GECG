#pragma once
#include "DirectXCommon.h"

class Object3dCommon {
public:
    // 初期化
    void Initialize(DirectXCommon* dxCommon);
    // 共通描画設定
    void CommonDrawSettings();

    DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
    DirectXCommon* dxCommon_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultLightResource_;
};