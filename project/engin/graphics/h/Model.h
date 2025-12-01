#pragma once
#include "DirectXCommon.h"
#include "MakeAffine.h"
#include <string>
#include <vector>

class Object3dCommon;

class Model {
public:
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };

    // 初期化
    void Initialize(Object3dCommon* object3dCommon, const std::string& textureFilePath);

    // 描画
    void Draw(Object3dCommon* object3dCommon);

private:
    Object3dCommon* object3dCommon_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ {};

    std::string textureFilePath_;
    uint32_t vertexCount_ = 0;
};