#include "Model.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include <cmath>

void Model::Initialize(Object3dCommon* object3dCommon, const std::string& textureFilePath)
{
    object3dCommon_ = object3dCommon;
    textureFilePath_ = textureFilePath;

    // テクスチャ読み込み
    TextureManager::GetInstance()->LoadTexture(textureFilePath);

    // 球体データの生成
    std::vector<VertexData> vertices;
    const int subdivision = 16;
    const float radius = 1.0f;
    const float kPi = 3.14159265358979323846f;
    const float kTwoPi = kPi * 2.0f;

    for (int lat = 0; lat < subdivision; ++lat) {
        float lat0 = kPi * (-0.5f + float(lat) / subdivision);
        float lat1 = kPi * (-0.5f + float(lat + 1) / subdivision);
        float y0 = sinf(lat0);
        float r0 = cosf(lat0);
        float y1 = sinf(lat1);
        float r1 = cosf(lat1);

        for (int lon = 0; lon < subdivision; ++lon) {
            float lon0 = kTwoPi * float(lon) / subdivision;
            float lon1 = kTwoPi * float(lon + 1) / subdivision;
            float x0 = cosf(lon0);
            float z0 = sinf(lon0);
            float x1 = cosf(lon1);
            float z1 = sinf(lon1);

            // 頂点4つ
            VertexData v0, v1, v2, v3;
            v0.position = { r0 * x0 * radius, y0 * radius, r0 * z0 * radius, 1.0f };
            v0.normal = { v0.position.x, v0.position.y, v0.position.z };
            v0.texcoord = { float(lon) / subdivision, 1.0f - float(lat) / subdivision };

            v1.position = { r0 * x1 * radius, y0 * radius, r0 * z1 * radius, 1.0f };
            v1.normal = { v1.position.x, v1.position.y, v1.position.z };
            v1.texcoord = { float(lon + 1) / subdivision, 1.0f - float(lat) / subdivision };

            v2.position = { r1 * x0 * radius, y1 * radius, r1 * z0 * radius, 1.0f };
            v2.normal = { v2.position.x, v2.position.y, v2.position.z };
            v2.texcoord = { float(lon) / subdivision, 1.0f - float(lat + 1) / subdivision };

            v3.position = { r1 * x1 * radius, y1 * radius, r1 * z1 * radius, 1.0f };
            v3.normal = { v3.position.x, v3.position.y, v3.position.z };
            v3.texcoord = { float(lon + 1) / subdivision, 1.0f - float(lat + 1) / subdivision };

            // 三角形2つ (v0, v2, v1) (v1, v2, v3)
            vertices.push_back(v0);
            vertices.push_back(v2);
            vertices.push_back(v1);
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
        }
    }
    vertexCount_ = static_cast<uint32_t>(vertices.size());

    // 頂点バッファの作成
    ID3D12Device* device = object3dCommon_->GetDxCommon()->GetDevice();
    size_t sizeInBytes = sizeof(VertexData) * vertices.size();

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

    device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&vertexResource_));

    // 頂点データのコピー
    VertexData* data = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&data));
    std::copy(vertices.begin(), vertices.end(), data);
    vertexResource_->Unmap(0, nullptr);

    // VBビュー作成
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeInBytes);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Model::Draw(Object3dCommon* object3dCommon)
{
    ID3D12GraphicsCommandList* commandList = object3dCommon->GetDxCommon()->GetCommandList();

    // 頂点バッファをセット
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    // テクスチャをセット
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_);
    commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);

    // 描画
    commandList->DrawInstanced(vertexCount_, 1, 0, 0);
}