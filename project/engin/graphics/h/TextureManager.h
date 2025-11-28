#pragma once
#include "DirectXCommon.h"
#include <d3d12.h>
#include <map>
#include <string>
#include <wrl.h>
#include "DirectXTex.h"

class TextureManager {
public:
    // シングルトンインスタンスの取得
    static TextureManager* GetInstance();

    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // テクスチャの読み込み
    void LoadTexture(const std::string& filePath);

    // SRVインデックスの取得
    uint32_t GetTextureIndexByFilePath(const std::string& filePath);

    // テクスチャのメタデータ取得
    const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

    // SRVハンドルの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

private:
    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // テクスチャデータ
    struct TextureData {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSrv;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSrv;
        DirectX::TexMetadata metadata;
    };

    DirectXCommon* dxCommon_ = nullptr;
    // テクスチャデータ一覧
    std::map<std::string, TextureData> textureDatas_;
    // ディスクリプタヒープの何番目を使っているか
    uint32_t srvIndex_ = 0; 
};