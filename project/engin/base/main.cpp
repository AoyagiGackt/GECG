// --------------------------------------------------
// include
// --------------------------------------------------
#include "Camera.h"
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
#include "TextureManager.h"

// 3D関連
#include "Model.h"
#include "ModelCommon.h"
#include "ModelManager.h"
#include "Object3d.h"
#include "Object3dCommon.h"

// パーティクル関連
#include "ParticleEmitter.h"
#include "ParticleManager.h"

#include "ImguiControl.h"
#include "LightingMode.h"
#include "MaterialManager.h"
#include "MeshManager.h"

#include "WinApp.h"
#include <SrvManager.h>
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
// グローバル変数の定義
// --------------------------------------------------
MeshManager meshManager;
MaterialManager materialManager;
int lightingMode = LightingMode::Lighting_HalfLambert;

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

    // CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    DirectXCommon* dxCommon = nullptr;
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    // SRVマネージャー初期化
    SrvManager::GetInstance()->Initialize(dxCommon);

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
    sprite1->SetPosition({ 100.0f, 100.0f });

    Sprite* sprite2 = new Sprite();
    sprite2->Initialize(spriteCommon, "Resources/monsterBall.png");
    sprite2->SetPosition({ 600.0f, 200.0f });

    // --------------------------------------------------
    // 3Dの初期化
    // --------------------------------------------------

    // 描画共通設定
    ModelCommon* modelCommon = new ModelCommon();
    modelCommon->Initialize(dxCommon);

    // 環境共通設定
    Object3dCommon* object3dCommon = new Object3dCommon();
    object3dCommon->Initialize(dxCommon);

    // モデルマネージャー初期化
    ModelManager::GetInstance()->Initialize(modelCommon);

    // モデルデータのロード
    ModelManager::GetInstance()->LoadModel("Resources/plane.obj", "Resources/uvChecker.png");

    // --------------------------------------------------
    // パーティクルの初期化
    // --------------------------------------------------

    ParticleManager* particleManager = ParticleManager::GetInstance();
    particleManager->Initialize(dxCommon);

    Model* particleModel = ModelManager::GetInstance()->FindModel("Resources/plane.obj");
    particleManager->SetModel(particleModel);

    particleManager->CreateParticleGroup("fire", "Resources/uvChecker.png");

    Vector3 emitterPos = { 0.0f, 0.0f, 0.0f };
    Transform emitterTransform = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, emitterPos };
    ParticleEmitter* fireEmitter = new ParticleEmitter("fire", emitterTransform);

    // --------------------------------------------------
    // カメラの生成
    // --------------------------------------------------

    Camera* camera = new Camera();
    camera->SetTranslate({ 0.0f, 0.0f, -10.0f });
    camera->SetRotate({ 0.0f, 0.0f, 0.0f });

    Object3d::SetCommonCamera(camera);

    // --------------------------------------------------
    // オブジェクト設定
    // --------------------------------------------------

    // 1つ目のオブジェクト
    Object3d* obj1 = new Object3d();
    obj1->Initialize(modelCommon);
    obj1->SetModel("Resources/plane.obj");
    obj1->SetPosition({ -2.0f, 0.0f, 0.0f }); // 左に配置

    // 2つ目のオブジェクト
    Object3d* obj2 = new Object3d();
    obj2->Initialize(modelCommon);
    obj2->SetModel("Resources/plane.obj");
    obj2->SetPosition({ 2.0f, 0.0f, 0.0f }); // 右に配置

    // 回転アニメーション用の変数
    Vector3 transformRotate = { 0.0f, 0.0f, 0.0f };
    Vector3 transformScale = { 1.0f, 1.0f, 1.0f };
    Vector3 transformTranslate = { 0.0f, 0.0f, 0.0f };

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

            // くるくる回転
            transformRotate.y += 0.02f;

            fireEmitter->Update();

            particleManager->Update(camera);

            // ImGui開始
            imguiManager->Begin();

            // カメラの更新
            camera->Update();

            // スプライト用 ImGui
            Vector2 pos = sprite1->GetPosition();
            float rot = sprite1->GetRotation();
            Vector2 size = sprite1->GetSize();
            Vector2 anchor = sprite1->GetAnchorPoint();
            Vector4 color = sprite1->GetColor();
            bool flipX = sprite1->GetFlipX();
            bool flipY = sprite1->GetFlipY();
            Vector2 texLT = sprite1->GetTextureLeftTop();
            Vector2 texSz = sprite1->GetTextureSize();

            #ifdef USE_IMGUI

            ImGui::SetNextWindowSize(ImVec2(500, 100), ImGuiCond_FirstUseEver);

            ImGui::Begin("Sprite Control");

            ImGui::SliderFloat2("Position", &pos.x, 0.0f, 1280.0f, "%.1f");

            ImGui::End();

            #endif

            sprite1->SetPosition(pos);
            sprite1->SetRotation(rot);
            sprite1->SetSize(size);
            sprite1->Update();
            sprite2->Update();

            // オブジェクトへの反映
            obj1->SetScale(transformScale);
            obj1->SetRotation(transformRotate);
            obj1->SetPosition(transformTranslate);

            obj2->SetRotation({ 0.0f, -transformRotate.y, 0.0f });

            // 行列更新
            obj1->Update();
            obj2->Update();

            imguiManager->End();

            // --------------------------------------------------
            // 描画処理
            // --------------------------------------------------

            dxCommon->PreDraw();

            SrvManager::GetInstance()->PreDraw();

            // スプライト描画
            spriteCommon->CommonDrawSettings();

            sprite1->Draw();
            //  sprite2->Draw();

            // 3Dオブジェクト描画

            // パイプラインステートなどのセット
            modelCommon->CommonDrawSettings();

            // ライトのセット
            object3dCommon->SetDefaultLight(dxCommon->GetCommandList());

            // 3dオブジェクト描画
            // obj1->Draw();
            // obj2->Draw();

            // パーティクル描画
            // particleManager->Draw(camera);

            // ImGui描画
            imguiManager->Draw(dxCommon);

            dxCommon->PostDraw();
        }
    }

    Logger::Log("Game Loop Finished.\n");

    // 終了処理
    imguiManager->Finalize();
    SrvManager::GetInstance()->Finalize();
    ModelManager::GetInstance()->Finalize();

    // 解放
    delete imguiManager;
    delete sprite1;
    delete sprite2;
    delete spriteCommon;

    delete fireEmitter;

    delete obj1;
    delete obj2;

    delete object3dCommon;
    delete modelCommon;

    delete input;
    delete dxCommon;
    delete winApp;
    delete camera;

    return 0;
}