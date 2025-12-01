#pragma once
#include "MakeAffine.h"

class Camera {
public:
    // コンストラクタ
    Camera();

    // 更新
    void Update();

    void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
    void SetFovY(float fovY) { fovY_ = fovY; }
    void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
    void SetNearClip(float nearClip) { nearClip_ = nearClip; }
    void SetFarClip(float farClip) { farClip_ = farClip; }

    const Vector3& GetRotate() const { return transform_.rotate; }
    const Vector3& GetTranslate() const { return transform_.translate; }
    const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
    const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }

private:
    Transform transform_;
    Matrix4x4 viewMatrix_;
    Matrix4x4 projectionMatrix_;

    // プロジェクション行列用パラメータ
    float fovY_;
    float aspectRatio_;
    float nearClip_;
    float farClip_;
};