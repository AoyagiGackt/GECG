#pragma once
#include "DirectXCommon.h"

class Object3dCommon {
public:
    void Initialize(DirectXCommon* dxCommon);
    void SetDefaultLight(ID3D12GraphicsCommandList* commandList);

private:
    DirectXCommon* dxCommon_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultLightResource_;
};