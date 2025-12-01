#include "Sprite.h"
#include "TextureManager.h"
#include "Logger.h"

using namespace Microsoft::WRL;

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
    spriteCommon_ = spriteCommon;
    textureFilePath_ = textureFilePath;

    // テクスチャ読み込み (TextureManagerに任せる)
    TextureManager::GetInstance()->LoadTexture(textureFilePath);
    
    // 初期サイズをテクスチャサイズに合わせる
    const auto& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath);
    size_ = { static_cast<float>(metadata.width), static_cast<float>(metadata.height) };
    textureSize_ = size_; // 切り出しサイズも全体に設定

    ID3D12Device* device = spriteCommon_->GetDevice();

    // 頂点バッファ作成
    vertexResource_ = CreateBufferResource(device, sizeof(VertexDataSprite) * 6);
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexDataSprite) * 6;
    vertexBufferView_.StrideInBytes = sizeof(VertexDataSprite);

    VertexDataSprite* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    // 左下 (0, 1)
    vertexData[0].position = { 0.0f, 1.0f, 0.5f, 1.0f };
    vertexData[0].texcoord = { 0.0f, 1.0f };
    vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
    // 左上 (0, 0)
    vertexData[1].position = { 0.0f, 0.0f, 0.5f, 1.0f };
    vertexData[1].texcoord = { 0.0f, 0.0f };
    vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
    // 右下 (1, 1)
    vertexData[2].position = { 1.0f, 1.0f, 0.5f, 1.0f };
    vertexData[2].texcoord = { 1.0f, 1.0f };
    vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
    // 左上
    vertexData[3] = vertexData[1];
    // 右上 (1, 0)
    vertexData[4].position = { 1.0f, 0.0f, 0.5f, 1.0f };
    vertexData[4].texcoord = { 1.0f, 0.0f };
    vertexData[4].normal = { 0.0f, 0.0f, -1.0f };
    // 右下
    vertexData[5] = vertexData[2];

    // マテリアル作成
    materialResource_ = CreateBufferResource(device, sizeof(MaterialSprite));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = false;
    materialData_->shadingType = 0;
    materialData_->uvTransform = MakeIdentity4x4();

    // WVP作成
    transformationMatrixResource_ = CreateBufferResource(device, sizeof(TransformationMatrixSprite));
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    *transformationMatrixData_ = { MakeIdentity4x4(), MakeIdentity4x4() };
}

void Sprite::SetTexture(std::string textureFilePath) {
    textureFilePath_ = textureFilePath;
    TextureManager::GetInstance()->LoadTexture(textureFilePath);
}

void Sprite::AdjustTextureSize() {
    // テクスチャのメタデータを取得
    const auto& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
    
    // UV座標空間での開始位置とサイズを計算
    Vector2 uvPos = { textureLeftTop_.x / metadata.width, textureLeftTop_.y / metadata.height };
    Vector2 uvSize = { textureSize_.x / metadata.width, textureSize_.y / metadata.height };

    // 反転処理
    if (isFlipX_) {
        uvPos.x += uvSize.x;
        uvSize.x = -uvSize.x;
    }
    if (isFlipY_) {
        uvPos.y += uvSize.y;
        uvSize.y = -uvSize.y;
    }

    // UV Transform行列の作成
    Matrix4x4 uvMatrix = MakeIdentity4x4();
    uvMatrix = Multiply(uvMatrix, MakeScaleMatrix({ uvSize.x, uvSize.y, 1.0f }));
    uvMatrix = Multiply(uvMatrix, MakeTranslateMatrix({ uvPos.x, uvPos.y, 0.0f }));
    
    materialData_->uvTransform = uvMatrix;
}

void Sprite::Update()
{
    AdjustTextureSize();

    // ワールド行列の計算
    Matrix4x4 anchorMatrix = MakeTranslateMatrix({ -anchorPoint_.x, -anchorPoint_.y, 0.0f });

    // スケーリング
    Matrix4x4 scaleMatrix = MakeScaleMatrix({ size_.x, size_.y, 1.0f });

    // 回転 (Z軸回転)
    Matrix4x4 rotateMatrix = MakeRotateZMatrix(rotation_);

    // 平行移動 (指定座標へ)
    Matrix4x4 translateMatrix = MakeTranslateMatrix({ position_.x, position_.y, 0.0f });

    // 行列の合成: Anchor -> Scale -> Rotate -> Translate
    Matrix4x4 worldMatrix = MakeIdentity4x4();
    worldMatrix = Multiply(worldMatrix, anchorMatrix);
    worldMatrix = Multiply(worldMatrix, scaleMatrix);
    worldMatrix = Multiply(worldMatrix, rotateMatrix);
    worldMatrix = Multiply(worldMatrix, translateMatrix);

    // ビュー・プロジェクション
    Matrix4x4 viewMatrix = MakeIdentity4x4();
    // 画面サイズ 1280x720 の正射影行列 (左上原点)
    Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);

    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;
}

void Sprite::Draw()
{
    ID3D12GraphicsCommandList* commandList = spriteCommon_->GetCommandList();

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    // マテリアル (色・UV)
    commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    // 座標変換
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
    
    // テクスチャ (TextureManagerからハンドルを取得してセット)
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_);
    commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);

    commandList->DrawInstanced(6, 1, 0, 0);
}

ComPtr<ID3D12Resource> Sprite::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
{
    D3D12_HEAP_PROPERTIES uploadHeapProperties {};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC resourceDesc {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = sizeInBytes;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtr<ID3D12Resource> resource = nullptr;
    device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&resource));
    return resource;
}