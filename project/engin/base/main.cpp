// --------------------------------------------------
// include
// --------------------------------------------------
#include "D3DResourceLeakChecker.h"
#include "DirectXCommon.h"
#include "DirectXTex.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "Logger.h"
#include "MakeAffine.h"
#include "ResourceObject.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "StringUtlity.h"
#include "WinApp.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include <Windows.h>
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
#include <wrl/client.h>

// --------------------------------------------------
// ライブラリのリンク
// --------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xinput.lib")

// --------------------------------------------------
// using declarations
// --------------------------------------------------
using namespace std::numbers;
using Microsoft::WRL::ComPtr;

// --------------------------------------------------
// 関数定義
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

    // 入力システム初期化
    Input* input = nullptr;
    input = new Input();
    input->Initialize(winApp);

    // --------------------------------------------------
    // スプライトシステムの初期化
    // --------------------------------------------------

    // スプライト共通設定の初期化
    SpriteCommon* spriteCommon = new SpriteCommon();
    spriteCommon->Initialize(dxCommon);

    // スプライトの初期化
    Sprite* sprite = new Sprite();
    sprite->Initialize(spriteCommon);

    // --------------------------------------------------
    // テクスチャのロード処理
    // --------------------------------------------------
    ID3D12Device* device = dxCommon->GetDevice();

    // 読み込むテクスチャファイル名
    std::string textureFile = "Resources/uvChecker.png";

    // 画像読み込み
    DirectX::ScratchImage mipImages = LoadTexture(textureFile);
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    // リソース作成
    ComPtr<ID3D12Resource> textureResource = CreateTextureResourse(device, metadata);
    // データ転送
    UploadTextureData(textureResource.Get(), mipImages);

    // SRVヒープのハンドル取得
    ID3D12DescriptorHeap* srvDescriptorHeap = dxCommon->GetSrvDescriptorHeap();
    UINT srvIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

    srvHandleCPU.ptr += srvIncrement;
    srvHandleGPU.ptr += srvIncrement;

    // シェーダリソースビュー(SRV)作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
    device->CreateShaderResourceView(textureResource.Get(), &srvDesc, srvHandleCPU);

    // Spriteにテクスチャをセット
    sprite->SetTextureHandle(srvHandleGPU);

    // --------------------------------------------------
    // ImGuiの初期化
    // --------------------------------------------------
    ImGuiManager* imguiManager = new ImGuiManager();
    imguiManager->Initialize(winApp, dxCommon);

    // --------------------------------------------------
    // メインループ
    // --------------------------------------------------

    while (true) {
        if (winApp->ProcessMessage()) {
            break;
        } else {

            // --------------------------------------------------
            // 更新処理
            // --------------------------------------------------

            // 入力更新
            input->Update();

            // ImGui受付開始
            imguiManager->Begin();

            // スプライト更新
            sprite->Update();

            ImGui::ShowDemoWindow();

            ImGui::Begin("Sprite Control");
            
            // クラスから現在の値を取得
            Vector3 spritePos = sprite->GetTranslate();
            Vector3 spriteRot = sprite->GetRotate();
            Vector3 spriteScale = sprite->GetScale();

            // ImGuiで値を変更
            ImGui::DragFloat3("Position", &spritePos.x, 1.0f);
            ImGui::DragFloat3("Rotation", &spriteRot.x, 0.01f);
            ImGui::DragFloat3("Scale", &spriteScale.x, 0.01f);

            // 変更した値をクラスにセット
            sprite->SetTranslate(spritePos);
            sprite->SetRotate(spriteRot);
            sprite->SetScale(spriteScale);

            ImGui::End();

            // ImGui内部コマンド生成
            imguiManager->End();

            // --------------------------------------------------
            // 描画処理
            // --------------------------------------------------
            
            dxCommon->PreDraw();

            // SRV用ディスクリプタヒープを設定
            ID3D12DescriptorHeap* descriptorHeaps[] = { dxCommon->GetSrvDescriptorHeap() };
            dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);

            // スプライト共通設定
            spriteCommon->CommonDrawSettings();

            // スプライト描画
            sprite->Draw();

            // ImGui描画
            imguiManager->Draw(dxCommon);

            // 描画後処理 (画面フリップなど)
            dxCommon->PostDraw();
        }
    }

    Logger::Log("Game Loop Finished.\n");

    // --- 終了処理 ---
    imguiManager->Finalize();

    // 生成と逆順に解放
    delete imguiManager;
    delete sprite;
    delete spriteCommon;
    delete input;
    delete dxCommon;
    delete winApp;

    return 0;
}