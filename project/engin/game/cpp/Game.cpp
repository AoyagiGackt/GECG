#include "Game.h"
#include "ModelManager.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "LightingMode.h"
#include <MeshManager.h>
#include <MaterialManager.h>

// --------------------------------------------------
// グローバル変数の定義
// --------------------------------------------------
MeshManager meshManager;
MaterialManager materialManager;
int lightingMode = LightingMode::Lighting_HalfLambert;

void Game::Initialize()
{
    winApp_ = new WinApp();
    winApp_->Initialize();

    dxCommon_ = new DirectXCommon();
    dxCommon_->Initialize(winApp_);

    // マネージャー系の初期化
    SrvManager::GetInstance()->Initialize(dxCommon_);
    TextureManager::GetInstance()->Initialize(dxCommon_);

    input_ = new Input();
    input_->Initialize(winApp_);

    audio_ = new Audio();
    audio_->Initialize();

    imguiManager_ = new ImGuiManager();
    imguiManager_->Initialize(winApp_, dxCommon_);

    // 描画共通系の初期化
    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);

    modelCommon_ = new ModelCommon();
    modelCommon_->Initialize(dxCommon_);

    object3dCommon_ = new Object3dCommon();
    object3dCommon_->Initialize(dxCommon_);

    ModelManager::GetInstance()->Initialize(modelCommon_);

    // --- アセット読み込み・オブジェクト生成 ---
    camera_ = new Camera();
    Object3d::SetCommonCamera(camera_);

    sprite1_ = new Sprite();
    sprite1_->Initialize(spriteCommon_, "Resources/uvChecker.png");
    sprite1_->SetPosition({ 100.0f, 100.0f });

    bgmData_ = audio_->LoadAudio("Resources/461_BPM174.wav");
}

void Game::Update()
{
    if (winApp_->ProcessMessage()) {
        endRequest_ = true;
        return;
    }

    input_->Update();
    imguiManager_->Begin();
    camera_->Update();

    // サウンド再生テスト
    if (input_->TriggerKey(DIK_SPACE)) {
        audio_->PlayWave(bgmData_);
    }

    // ImGui
#ifdef USE_IMGUI
    Vector2 pos = sprite1_->GetPosition();
    ImGui::SetNextWindowSize(ImVec2(500, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Sprite Control");
    ImGui::SliderFloat2("Position", &pos.x, 0.0f, 1280.0f, "%.1f");
    ImGui::End();
    sprite1_->SetPosition(pos);
#endif

    sprite1_->Update();
    imguiManager_->End();
}

void Game::Draw()
{
    dxCommon_->PreDraw();
    SrvManager::GetInstance()->PreDraw();

    // スプライト描画
    spriteCommon_->CommonDrawSettings();
    sprite1_->Draw();

    // ImGui描画
    imguiManager_->Draw(dxCommon_);

    dxCommon_->PostDraw();
}

void Game::Finalize()
{
    // 解放
    imguiManager_->Finalize();
    audio_->Finalize();
    ModelManager::GetInstance()->Finalize();

    delete sprite1_;
    delete camera_;
    delete imguiManager_;
    delete audio_;
    delete input_;
    delete spriteCommon_;
    delete modelCommon_;
    delete object3dCommon_;
    delete dxCommon_;
    delete winApp_;
}