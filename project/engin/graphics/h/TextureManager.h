#pragma once
#include "DirectXCommon.h"
#include "DirectXTex.h"
#include <d3d12.h>
#include <map>
#include <string>
#include <wrl.h>

class TextureManager {
public:
    static TextureManager* GetInstance();
    void Initialize(DirectXCommon* dxCommon);

    void Finalize();

    // ロード
    void LoadTexture(const std::string& filePath);

    // テクスチャのSRVインデックス取得
    uint32_t GetTextureIndexByFilePath(const std::string& filePath);

    // GPUハンドル取得 (描画で使う)
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

    const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

private:
    struct TextureData {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        uint32_t srvIndex; // ヒープ上のインデックス
        DirectX::TexMetadata metadata;
    };

    DirectXCommon* dxCommon_ = nullptr;
    // テクスチャデータ一覧
    std::map<std::string, TextureData> textureDatas_;
};