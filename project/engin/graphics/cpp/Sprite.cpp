#include "Sprite.h"
#include "DirectXTex.h"
#include "StringUtlity.h"

using namespace Microsoft::WRL;

namespace {
DirectX::ScratchImage LoadTextureInternal(const std::string& filePath)
{
    DirectX::ScratchImage image {};
    std::wstring filePathW = StringUtility::ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));
    DirectX::ScratchImage mipImages {};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 8, mipImages);
    assert(SUCCEEDED(hr));
    return mipImages;
}

ComPtr<ID3D12Resource> CreateTextureResourceInternal(ID3D12Device* device, const DirectX::TexMetadata& metadata)
{
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

    ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource));
    assert(SUCCEEDED(hr));
    return resource;
}

void UploadTextureDataInternal(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
{
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
        const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
        HRESULT hr = texture->WriteToSubresource(
            UINT(mipLevel),
            nullptr,
            img->pixels,
            UINT(img->rowPitch),
            UINT(img->slicePitch));
        assert(SUCCEEDED(hr));
    }
}
}

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
    assert(spriteCommon);
    spriteCommon_ = spriteCommon;
    ID3D12Device* device = spriteCommon_->GetDevice();

    // 頂点バッファの作成
    vertexResource_ = CreateBufferResource(device, sizeof(VertexDataSprite) * 6);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexDataSprite) * 6;
    vertexBufferView_.StrideInBytes = sizeof(VertexDataSprite);

    VertexDataSprite* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    vertexData[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };
    vertexData[0].texcoord = { 0.0f, 1.0f };
    vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
    vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
    vertexData[1].texcoord = { 0.0f, 0.0f };
    vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
    vertexData[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };
    vertexData[2].texcoord = { 1.0f, 1.0f };
    vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
    vertexData[3].position = { 0.0f, 0.0f, 0.0f, 1.0f };
    vertexData[3].texcoord = { 0.0f, 0.0f };
    vertexData[3].normal = { 0.0f, 0.0f, -1.0f };
    vertexData[4].position = { 640.0f, 0.0f, 0.0f, 1.0f };
    vertexData[4].texcoord = { 1.0f, 0.0f };
    vertexData[4].normal = { 0.0f, 0.0f, -1.0f };
    vertexData[5].position = { 640.0f, 360.0f, 0.0f, 1.0f };
    vertexData[5].texcoord = { 1.0f, 1.0f };
    vertexData[5].normal = { 0.0f, 0.0f, -1.0f };

    // マテリアルリソースの作成
    materialResource_ = CreateBufferResource(device, sizeof(MaterialSprite));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = false;
    materialData_->uvTransform = MakeIdentity4x4();

    // 座標変換行列リソースの作成
    transformationMatrixResource_ = CreateBufferResource(device, sizeof(TransformationMatrixSprite));
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    *transformationMatrixData_ = { MakeIdentity4x4(), MakeIdentity4x4() };
}

// テクスチャ読み込みとSRV作成
void Sprite::LoadTexture(const std::string& filePath, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    ID3D12Device* device = spriteCommon_->GetDevice();

    // 画像ファイルを読み込む
    DirectX::ScratchImage mipImages = LoadTextureInternal(filePath);
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    // リソースを作成してデータを転送
    textureResource_ = CreateTextureResourceInternal(device, metadata);
    UploadTextureDataInternal(textureResource_.Get(), mipImages);

    // SRVを作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    // 指定されたヒープの場所にSRVを作る
    device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, cpuHandle);

    // ハンドルを自分の中に保存
    textureSrvHandle_ = gpuHandle;
}

void Sprite::Update()
{
    // 行列計算
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 viewMatrix = MakeIdentity4x4();
    Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;
}

void Sprite::Draw()
{
    ID3D12GraphicsCommandList* commandList = spriteCommon_->GetCommandList();

    // 頂点バッファの設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    // ルートパラメータの設定
    // Material
    commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    // TransformationMatrix
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
    // Texture
    commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle_);

    // 描画
    commandList->DrawInstanced(6, 1, 0, 0);
}

ComPtr<ID3D12Resource> Sprite::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
{
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
    ComPtr<ID3D12Resource> resource = nullptr;
    device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&resource));
    return resource;
}