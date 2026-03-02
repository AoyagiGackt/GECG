#include "Object3d.h"
#include "ModelCommon.h"
#include "ModelManager.h"
#include "LightManager.h"
#include "Camera.h"
#include <cmath>

using namespace Microsoft::WRL;

Camera* Object3d::commonCamera_ = nullptr;

void Object3d::SetCommonCamera(Camera* camera)
{
    commonCamera_ = camera;
}

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

    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;

    // カメラがあればカメラから行列をもらう 
    Camera* camera = camera_ ? camera_ : commonCamera_;

    if (camera) {
        viewMatrix = camera->GetViewMatrix();
        projectionMatrix = camera->GetProjectionMatrix();
    }
    
    // WVP行列の合成
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

    // 定数バッファへ転送
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;

    // 毎フレームライティングモードをマテリアルに反映させる
    materialData_->shadingType = LightManager::GetInstance()->GetLightingMode();
}

void Object3d::SetModel(const std::string& filePath)
{
    // マネージャーからモデルを検索してセット
    model_ = ModelManager::GetInstance()->FindModel(filePath);
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

    model_->Draw(modelCommon_);
}