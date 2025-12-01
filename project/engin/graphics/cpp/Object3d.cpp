#include "Object3d.h"
#include "ModelCommon.h"
#include "ModelManager.h"
#include <cmath>

using namespace Microsoft::WRL;

void Object3d::Initialize(ModelCommon* modelCommon)
{
    modelCommon_ = modelCommon;
    ID3D12Device* device = modelCommon_->GetDxCommon()->GetDevice();

    // Transform用リソース作成
    D3D12_HEAP_PROPERTIES heapProps { D3D12_HEAP_TYPE_UPLOAD };
    D3D12_RESOURCE_DESC resDesc {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = sizeof(TransformationMatrix);
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&transformationMatrixResource_));
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    *transformationMatrixData_ = { MakeIdentity4x4(), MakeIdentity4x4() };

    // Material用リソース作成
    resDesc.Width = sizeof(Material);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&materialResource_));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = true;
    materialData_->shadingType = 1;
    materialData_->uvTransform = MakeIdentity4x4();
}

void Object3d::Update()
{
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 viewMatrix = MakeIdentity4x4();
    viewMatrix.m[3][2] = -10.0f;
    viewMatrix = Inverse(viewMatrix);
    Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;
}

void Object3d::Draw()
{
    if (!model_)
        return;

    // ModelCommonからコマンドリストを取得
    ID3D12GraphicsCommandList* commandList = modelCommon_->GetDxCommon()->GetCommandList();

    // マテリアルと座標変換を設定
    commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

    // モデルの描画呼び出し
    model_->Draw(modelCommon_);
}

void Object3d::SetModel(const std::string& filePath)
{
    // マネージャーからモデルを検索してセット
    model_ = ModelManager::GetInstance()->FindModel(filePath);
}