#include "Object3dCommon.h"
#include "MakeAffine.h"

void Object3dCommon::Initialize(DirectXCommon* dxCommon)
{
    dxCommon_ = dxCommon;
    ID3D12Device* device = dxCommon_->GetDevice();

    // ライトリソースの作成
    D3D12_HEAP_PROPERTIES heapProps { D3D12_HEAP_TYPE_UPLOAD };
    D3D12_RESOURCE_DESC resDesc {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = 256; // サイズ
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&defaultLightResource_));

    // ライトデータの書き込み
    struct DirectionalLight {
        Vector4 color;
        Vector3 direction;
        float intensity;
    };
    DirectionalLight* lightData = nullptr;
    defaultLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightData));
    lightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lightData->direction = { 0.0f, -1.0f, 1.0f }; // 斜め前から
    lightData->intensity = 1.0f;
    defaultLightResource_->Unmap(0, nullptr);
}

void Object3dCommon::SetDefaultLight(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetGraphicsRootConstantBufferView(3, defaultLightResource_->GetGPUVirtualAddress());
}