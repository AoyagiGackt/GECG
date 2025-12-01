// --------------------------------------------------
// include
// --------------------------------------------------
#include "D3DResourceLeakChecker.h"
#include "DirectXCommon.h"
#include "DirectXTex.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "Logger.h"
#include "TextureManager.h"
#include "MakeAffine.h"
#include "ResourceObject.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "StringUtlity.h"
#include "Object3dCommon.h"
#include "Model.h"
#include "Object3d.h"
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
    // スプライトの初期化
    // --------------------------------------------------

    // スプライト共通設定の初期化
    SpriteCommon* spriteCommon = new SpriteCommon();
    spriteCommon->Initialize(dxCommon);

    // TextureManager初期化
    TextureManager::GetInstance()->Initialize(dxCommon);

    // テクスチャロード 
    TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
    TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");

    // スプライト生成
    Sprite* sprite1 = new Sprite();
    sprite1->Initialize(spriteCommon, "Resources/uvChecker.png");
    sprite1->SetPosition({ 200.0f, 200.0f }); // 座標セット

    Sprite* sprite2 = new Sprite();
    sprite2->Initialize(spriteCommon, "Resources/monsterBall.png");
    sprite2->SetPosition({ 600.0f, 200.0f });

    // --------------------------------------------------
    // 3Dの初期化
    // --------------------------------------------------

    // 3D共通部の初期化
    Object3dCommon* object3dCommon = new Object3dCommon();
    object3dCommon->Initialize(dxCommon);

    // モデルデータの生成 (1つだけ)
    Model* model1 = new Model();
    model1->Initialize(object3dCommon, "Resources/monsterBall.png");

    // オブジェクトの生成 (モデルを使い回す)
    Object3d* obj1 = new Object3d();
    obj1->Initialize(object3dCommon);
    obj1->SetModel(model1);
    obj1->SetPosition({ -2.0f, 0.0f, 0.0f }); // 左に配置

    Object3d* obj2 = new Object3d();
    obj2->Initialize(object3dCommon);
    obj2->SetModel(model1); // 同じモデルをセット
    obj2->SetPosition({ 2.0f, 0.0f, 0.0f }); // 右に配置

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

            Vector2 pos = sprite1->GetPosition();
            float rot = sprite1->GetRotation();
            Vector2 size = sprite1->GetSize();
            Vector2 anchor = sprite1->GetAnchorPoint();
            Vector4 color = sprite1->GetColor();
            bool flipX = sprite1->GetFlipX();
            bool flipY = sprite1->GetFlipY();
            Vector2 texLT = sprite1->GetTextureLeftTop();
            Vector2 texSz = sprite1->GetTextureSize();

            ImGui::ShowDemoWindow();


            ImGui::Begin("Sprite 1 Control");
            ImGui::DragFloat2("Position", &pos.x, 1.0f);
            ImGui::SliderAngle("Rotation", &rot);
            ImGui::DragFloat2("Size", &size.x, 1.0f);
            ImGui::DragFloat2("Anchor", &anchor.x, 0.01f, 0.0f, 1.0f);
            ImGui::ColorEdit4("Color", &color.x);
            ImGui::Checkbox("Flip X", &flipX);
            ImGui::Checkbox("Flip Y", &flipY);
            ImGui::Text("Texture Cutout");
            ImGui::DragFloat2("Cut Pos", &texLT.x, 1.0f);
            ImGui::DragFloat2("Cut Size", &texSz.x, 1.0f);
            ImGui::End();

            // 値をセット
            sprite1->SetPosition(pos);
            sprite1->SetRotation(rot);
            sprite1->SetSize(size);
            sprite1->SetAnchorPoint(anchor);
            sprite1->SetColor(color);
            sprite1->SetFlipX(flipX);
            sprite1->SetFlipY(flipY);
            sprite1->SetTextureLeftTop(texLT);
            sprite1->SetTextureSize(texSz);

            // 更新
            sprite1->Update();
            sprite2->Update(); // 2つ目も更新

            obj1->Update();
            obj2->Update();

            imguiManager->End();

            // --- 描画 ---
            dxCommon->PreDraw();
            spriteCommon->CommonDrawSettings();

            //sprite1->Draw(); // 1つ目描画
            //sprite2->Draw(); // 2つ目描画

            object3dCommon->CommonDrawSettings(); // 3D用の共通設定
            obj1->Draw();
            obj2->Draw();

            imguiManager->Draw(dxCommon);
            dxCommon->PostDraw();
        }
    }

    Logger::Log("Game Loop Finished.\n");

    // 終了処理
    imguiManager->Finalize();

    // 解放
    delete imguiManager;
    delete sprite1;
    delete sprite2;
    delete spriteCommon;
    delete input;
    delete dxCommon;
    delete winApp;

    return 0;
}