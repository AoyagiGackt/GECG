#pragma once
#include "DirectXCommon.h"
#include "MakeAffine.h"
#include <string>
#include <vector>

class ModelCommon;

class Model {
public:
    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;
    };

    // 初期化
    void Initialize(ModelCommon* modelCommon, const std::string& modelFilePath, const std::string& textureFilePath);
    
    // 描画
    void Draw(ModelCommon* modelCommon);

private:
    // OBJファイル読み込み関数
    void LoadObjFile(const std::string& filePath);
    ModelCommon* modelCommon_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ {};

    std::string textureFilePath_;

    std::vector<VertexData> vertices_;
};