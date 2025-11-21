#include <Windows.h>
#include "WinApp.h"
#include "DirectXCommon.h"
#include "DirectXTex.h"
#include <wrl/client.h>
#include "Input.h"
#include "Logger.h"
#include "MakeAffine.h"
#include "ResourceObject.h"
#include "StringUtlity.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "D3DResourceLeakChecker.h"
#include "d3dx12.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "ImGuiManager.h"
#include <Xinput.h>
#include <cassert>
#include <cstdint>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <format>
#include <numbers>
#include <string>
#include <vector>

// --------------------------------------------------
// ライブラリのリンク
// --------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xinput.lib")

using namespace std::numbers;
using Microsoft::WRL::ComPtr;

// --------------------------------------------------
// 関数定義 (Texture関連はまだMainに残します)
// --------------------------------------------------

DirectX::ScratchImage LoadTexture(const std::string filePath)
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

ComPtr<ID3D12Resource> CreateTextureResourse(ID3D12Device* device, const DirectX::TexMetadata& metadata)
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

void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
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

// --------------------------------------------------
// メイン関数
// --------------------------------------------------

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    D3D12ResourceLeakChecker leakCheck;

    // ポインタ
    WinApp* winApp = nullptr;

    // windowsAPIの初期化
    winApp = new WinApp();
    winApp->Initialize();

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    DirectXCommon* dxCommon = nullptr;
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    // SpriteCommonとSpriteの初期化
    SpriteCommon* spriteCommon = new SpriteCommon();
    spriteCommon->Initialize(dxCommon);

    Sprite* sprite = new Sprite();
    sprite->Initialize(spriteCommon);

    // テクスチャ読み込み処理
    ID3D12Device* device = dxCommon->GetDevice();
    std::vector<std::string> textureFiles = { "Resources/uvChecker.png", "Resources/monsterBall.png" };
    std::vector<ComPtr<ID3D12Resource>> textureResources;
    std::vector<DirectX::ScratchImage> mipImagesList;

    // dxCommonからSRVヒープを取得
    ID3D12DescriptorHeap* srvDescriptorHeap = dxCommon->GetSrvDescriptorHeap();
    UINT srvIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

    // 先頭(ImGui用など)を避ける
    srvHandleCPU.ptr += srvIncrement;
    srvHandleGPU.ptr += srvIncrement;

    for (size_t i = 0; i < textureFiles.size(); ++i) {
        mipImagesList.push_back(LoadTexture(textureFiles[i]));
        const DirectX::TexMetadata& metadata = mipImagesList.back().GetMetadata();
        ComPtr<ID3D12Resource> texRes = CreateTextureResourse(device, metadata);
        UploadTextureData(texRes.Get(), mipImagesList.back());
        textureResources.push_back(texRes);
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
        srvDesc.Format = metadata.format;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
        device->CreateShaderResourceView(texRes.Get(), &srvDesc, srvHandleCPU);
        
        // 0番目のテクスチャをSpriteにセット
        if (i == 0) {
            sprite->SetTextureHandle(srvHandleGPU);
        }

        srvHandleCPU.ptr += srvIncrement;
        srvHandleGPU.ptr += srvIncrement;
    }

    // ImGuiの初期化
    ImGuiManager* imguiManager = new ImGuiManager();
    imguiManager->Initialize(winApp, dxCommon);

    Input* input = new Input();
    input->Initialize(winApp);

    // --------------------------------------------------
    // メインループ
    // --------------------------------------------------

    while (true) {
        if (winApp->ProcessMessage()) {
            break;
        } else {
            
            imguiManager->Begin();
            dxCommon->PreDraw();
            input->Update();

            // スプライトの更新（内部で行列計算などが行われます）
            sprite->Update();

            // ImGuiによる制御 (Spriteクラスの値を操作するように変更)
            ImGui::Begin("Sprite Control");
            // 現在の値を取得
            Vector3 pos = sprite->GetTranslate();
            Vector3 rot = sprite->GetRotate();
            Vector3 scale = sprite->GetScale();
            
            // ImGuiで操作
            ImGui::DragFloat3("Position", &pos.x, 1.0f);
            ImGui::DragFloat3("Rotation", &rot.x, 0.01f);
            ImGui::DragFloat3("Scale", &scale.x, 0.01f);
            
            // 変更した値をセット
            sprite->SetTranslate(pos);
            sprite->SetRotate(rot);
            sprite->SetScale(scale);
            ImGui::End();

            ID3D12DescriptorHeap* descriptorHeaps[] = { dxCommon->GetSrvDescriptorHeap() };
            dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);

            // 描画処理
            spriteCommon->CommonDrawSettings();
            // スプライト描画
            sprite->Draw();

            imguiManager->End();
            imguiManager->Draw(dxCommon);

            // 描画後処理
            dxCommon->PostDraw();
        }
    }

    Logger::Log("Hello, DirectX!\n");

    imguiManager->Finalize();
    winApp->Finalize();

    delete sprite;
    delete spriteCommon;
    delete input;
    delete imguiManager;
    delete dxCommon;
    delete winApp;

    return 0;
}