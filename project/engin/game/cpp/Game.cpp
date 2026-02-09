#include "Game.h"
#include "LightingMode.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "ModelManager.h"
#include "SrvManager.h"
#include "TextureManager.h"

// --------------------------------------------------
// グローバル変数の実体定義
// --------------------------------------------------
MeshManager meshManager;
MaterialManager materialManager;
int lightingMode = LightingMode::Lighting_HalfLambert;

// --------------------------------------------------
// 初期化
// --------------------------------------------------
void MyGame::Initialize()
{
    // 基盤の初期化
    Framework::Initialize();

    // 描画共通マネージャーの初期化
    TextureManager::GetInstance()->Initialize(dxCommon_);

    // 各共通設定の初期化
    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);

    modelCommon_ = new ModelCommon();
    modelCommon_->Initialize(dxCommon_);

    object3dCommon_ = new Object3dCommon();
    object3dCommon_->Initialize(dxCommon_);

    ModelManager::GetInstance()->Initialize(modelCommon_);

    // アセットの読み込み
    bgmData_ = audio_->LoadAudio("Resources/461_BPM174.wav");

    // カメラ
    camera_ = new Camera();
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });
    Object3d::SetCommonCamera(camera_);

    // スプライト
    sprite1_ = new Sprite();
    sprite1_->Initialize(spriteCommon_, "Resources/uvChecker.png");
    sprite1_->SetPosition({ 100.0f, 100.0f });
}

// --------------------------------------------------
// 更新処理
// --------------------------------------------------
void MyGame::Update() 
{
    Framework::Update();

    // サウンド再生
    if (input_->TriggerKey(DIK_SPACE)) {
        audio_->PlayWave(bgmData_);
    }

    camera_->Update();

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

// --------------------------------------------------
// 描画処理
// --------------------------------------------------
void MyGame::Draw()
{
    dxCommon_->PreDraw();
    SrvManager::GetInstance()->PreDraw();

    spriteCommon_->CommonDrawSettings();
    sprite1_->Draw();

    imguiManager_->Draw(dxCommon_);
    dxCommon_->PostDraw();
}

// --------------------------------------------------
// 解放
// --------------------------------------------------
void MyGame::Finalize()
{
    delete sprite1_;
    delete camera_;
    delete spriteCommon_;
    delete modelCommon_;
    delete object3dCommon_;

    ModelManager::GetInstance()->Finalize();

    Framework::Finalize();
}