#pragma once
#include "MakeAffine.h"
#include "Model.h"

class Object3dCommon;

class Object3d {
public:
    // 初期化
    void Initialize(Object3dCommon* object3dCommon);

    // 更新
    void Update();

    // 描画
    void Draw();

    // モデルのセット
    void SetModel(Model* model) { model_ = model; }

    // トランスフォーム操作
    void SetPosition(const Vector3& position) { transform_.translate = position; }
    void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }
    void SetScale(const Vector3& scale) { transform_.scale = scale; }
    const Transform& GetTransform() const { return transform_; }

private:
    // 定数バッファ用構造体
    struct TransformationMatrix {
        Matrix4x4 WVP;
        Matrix4x4 World;
    };
    struct Material {
        Vector4 color;
        int enableLighting;
        int shadingType;
        float padding[2];
        Matrix4x4 uvTransform;
    };

    Object3dCommon* object3dCommon_ = nullptr;
    Model* model_ = nullptr;

    Transform transform_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

    // リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
    TransformationMatrix* transformationMatrixData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;
};