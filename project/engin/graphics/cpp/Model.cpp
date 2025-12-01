#include "Model.h"
#include "ModelCommon.h"
#include "TextureManager.h"
#include <fstream>
#include <sstream>
#include <cmath>

using namespace Microsoft::WRL;

void Model::Initialize(ModelCommon* modelCommon, const std::string& modelFilePath, const std::string& textureFilePath)
{
    modelCommon_ = modelCommon;
    textureFilePath_ = textureFilePath;

    TextureManager::GetInstance()->LoadTexture(textureFilePath);
    LoadObjFile(modelFilePath);

    ID3D12Device* device = modelCommon_->GetDxCommon()->GetDevice();
    size_t sizeInBytes = sizeof(VertexData) * vertices_.size();

    D3D12_HEAP_PROPERTIES uploadHeapProperties { D3D12_HEAP_TYPE_UPLOAD };

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

    VertexData* data = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&data));
    std::copy(vertices_.begin(), vertices_.end(), data);
    vertexResource_->Unmap(0, nullptr);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeInBytes);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Model::Draw(ModelCommon* modelCommon)
{
    ID3D12GraphicsCommandList* commandList = modelCommon->GetDxCommon()->GetCommandList();

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_);
    commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);

    commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);
}

// OBJファイル読み込み
void Model::LoadObjFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    assert(file.is_open());

    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string identifier;
        ss >> identifier;

        if (identifier == "v") {
            Vector4 position;
            ss >> position.x >> position.y >> position.z;
            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifier == "vt") {
            Vector2 texcoord;
            ss >> texcoord.x >> texcoord.y;
            texcoord.y = 1.0f - texcoord.y;
            texcoords.push_back(texcoord);
        } else if (identifier == "vn") {
            Vector3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (identifier == "f") {
            // 面情報の読み込み
            VertexData triangle[3];
            for (int i = 0; i < 3; ++i) {
                std::string s;
                ss >> s;

                std::stringstream ss2(s);
                std::string indexStr;
                int indices[3] = { 0, 0, 0 };
                int count = 0;

                while (std::getline(ss2, indexStr, '/')) {
                    if (!indexStr.empty()) {
                        indices[count] = std::stoi(indexStr);
                    }
                    count++;
                }

                if (indices[0] > 0) {
                    triangle[i].position = positions[indices[0] - 1];
                }

                if (indices[1] > 0) {
                    triangle[i].texcoord = texcoords[indices[1] - 1];
                }

                if (indices[2] > 0) {
                    triangle[i].normal = normals[indices[2] - 1];
                }
            }

            // 頂点リストに追加
            vertices_.push_back(triangle[0]);
            vertices_.push_back(triangle[1]);
            vertices_.push_back(triangle[2]);
        }
    }
}