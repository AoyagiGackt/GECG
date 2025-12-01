#include "TextureManager.h"
#include "DirectXTex.h"
#include "StringUtlity.h"
#include <vector>
#include <cassert>

using namespace Microsoft::WRL;

TextureManager* TextureManager::GetInstance()
{
    static TextureManager instance;
    return &instance;
}

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
    dxCommon_ = dxCommon;
    srvIndex_ = 1;
}

void TextureManager::LoadTexture(const std::string& filePath)
{
    // 読み込み済みなら早期リターン
    if (textureDatas_.contains(filePath)) {
        return;
    }

    assert(dxCommon_);
    ID3D12Device* device = dxCommon_->GetDevice();

    // 画像ファイルを読み込む
    DirectX::ScratchImage image {};
    std::wstring filePathW = StringUtility::ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));

    DirectX::ScratchImage mipImages {};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 8, mipImages);
    assert(SUCCEEDED(hr));

    // リソース作成
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    D3D12_RESOURCE_DESC resourceDesc {};
    resourceDesc.Width = UINT(metadata.width);
    resourceDesc.Height = UINT(metadata.height);
    resourceDesc.MipLevels = UINT16(metadata.mipLevels);
    resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
    resourceDesc.Format = metadata.format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

    D3D12_HEAP_PROPERTIES heapProperties {};
    heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

    ComPtr<ID3D12Resource> resource;
    hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource));
    assert(SUCCEEDED(hr));

    // データ転送
    for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
        const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
        resource->WriteToSubresource(
            UINT(mipLevel),
            nullptr,
            img->pixels,
            UINT(img->rowPitch),
            UINT(img->slicePitch));
    }

    // SRV作成
    // ヒープ上の場所を決定
    ID3D12DescriptorHeap* srvHeap = dxCommon_->GetSrvDescriptorHeap();
    UINT srvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();

    cpuHandle.ptr += (srvIncrementSize * srvIndex_);
    gpuHandle.ptr += (srvIncrementSize * srvIndex_);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    device->CreateShaderResourceView(resource.Get(), &srvDesc, cpuHandle);

    // データ保存
    TextureData& data = textureDatas_[filePath];
    data.resource = resource;
    data.cpuDescHandleSrv = cpuHandle;
    data.gpuDescHandleSrv = gpuHandle;
    data.metadata = metadata;

    // 次のためにインデックスを進める
    srvIndex_++;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
    if (textureDatas_.contains(filePath)) {
        return 0;
    }
    assert(0);
    return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
    assert(textureDatas_.contains(filePath));
    return textureDatas_[filePath].gpuDescHandleSrv;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
    assert(textureDatas_.contains(filePath));
    return textureDatas_[filePath].metadata;
}