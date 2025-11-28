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
    
    // SRV用ヒープ
    ID3D12DescriptorHeap* srvHeap = dxCommon->GetSrvDescriptorHeap();
    UINT srvIncrement = dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvHeap->GetGPUDescriptorHandleForHeapStart();

    srvHandleCPU.ptr += srvIncrement;
    srvHandleGPU.ptr += srvIncrement;

    // Spriteにテクスチャをセット
    sprite->LoadTexture("Resources/uvChecker.png", srvHandleCPU, srvHandleGPU);

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

    // 終了処理
    imguiManager->Finalize();

    // 解放
    delete imguiManager;
    delete sprite;
    delete spriteCommon;
    delete input;
    delete dxCommon;
    delete winApp;

    return 0;
}