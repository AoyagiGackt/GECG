#pragma once
#include "MakeAffine.h"
#include "SpriteCommon.h"
#include <DirectXMath.h>

// 頂点データ構造体
struct VertexDataSprite {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

// マテリアル構造体
struct MaterialSprite {
    Vector4 color;
    int enableLighting;
    int shadingType;
    float padding[2];
    Matrix4x4 uvTransform;
};

// 座標変換行列構造体
struct TransformationMatrixSprite {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

class Sprite {
public:
    // 初期化
    void Initialize(SpriteCommon* spriteCommon);
    // 更新
    void Update();
    // 描画
    void Draw();

    const Vector3& GetTranslate() const { return transform_.translate; }
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

    const Vector3& GetRotate() const { return transform_.rotate; }
    void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }

    const Vector3& GetScale() const { return transform_.scale; }
    void SetScale(const Vector3& scale) { transform_.scale = scale; }

    void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) { textureSrvHandle_ = textureHandle; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

private:
    SpriteCommon* spriteCommon_ = nullptr;

    // リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ {};

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    MaterialSprite* materialData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
    TransformationMatrixSprite* transformationMatrixData_ = nullptr;

    // テクスチャハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle_ {};

    // トランスフォーム
    Transform transform_ { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
};