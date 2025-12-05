#pragma once
#include "MakeAffine.h"
#include "SpriteCommon.h"
#include <string>

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
    void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);
    // 更新
    void Update();
    // 描画
    void Draw();

    // 座標
    const Vector2& GetPosition() const { return position_; }
    void SetPosition(const Vector2& position) { position_ = position; }

    // 回転
    float GetRotation() const { return rotation_; }
    void SetRotation(float rotation) { rotation_ = rotation; }

    // サイズ
    const Vector2& GetSize() const { return size_; }
    void SetSize(const Vector2& size) { size_ = size; }

    // アンカーポイント
    const Vector2& GetAnchorPoint() const { return anchorPoint_; }
    void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

    // 色 (RGBA)
    const Vector4& GetColor() const { return materialData_->color; }
    void SetColor(const Vector4& color) { materialData_->color = color; }

    // 反転
    bool GetFlipX() const { return isFlipX_; }
    void SetFlipX(bool isFlipX) { isFlipX_ = isFlipX; }
    bool GetFlipY() const { return isFlipY_; }
    void SetFlipY(bool isFlipY) { isFlipY_ = isFlipY; }

    // テクスチャ範囲指定 (左上X, 左上Y, 幅, 高さ)
    const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }
    void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }
    const Vector2& GetTextureSize() const { return textureSize_; }
    void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

    // テクスチャ変更
    void SetTexture(std::string textureFilePath);

private:

    void AdjustTextureSize();

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

    // テクスチャ関連
    std::string textureFilePath_;

    // スプライトパラメータ
    Vector2 position_ = { 0.0f, 0.0f };
    float rotation_ = 0.0f;
    Vector2 size_ = { 640.0f, 360.0f };
    Vector2 anchorPoint_ = { 0.0f, 0.0f }; // デフォルトは左上
    bool isFlipX_ = false;
    bool isFlipY_ = false;

    // テクスチャ切り出し用
    Vector2 textureLeftTop_ = { 0.0f, 0.0f };
    Vector2 textureSize_ = { 100.0f, 100.0f };
};