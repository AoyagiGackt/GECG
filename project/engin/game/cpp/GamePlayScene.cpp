#include "GamePlayScene.h"
#include "ImguiManager.h"
#include "TextureManager.h"

void GamePlayScene::Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio)
{
    // 依存関係の保存
    dxCommon_ = dxCommon;
    input_ = input;
    audio_ = audio;

    // スプライト共通設定
    spriteCommon_ = new SpriteCommon();
    spriteCommon_->Initialize(dxCommon_);

    // アセットロード
    bgmData_ = audio_->LoadAudio("Resources/461_BPM174.wav");

    // オブジェクト生成
    camera_ = new Camera();
    sprite1_ = new Sprite();
    sprite1_->Initialize(spriteCommon_, "Resources/uvChecker.png");
    sprite1_->SetPosition({ 100.0f, 100.0f });
}

void GamePlayScene::Update()
{
    // ゲームロジック
    if (input_->TriggerKey(DIK_SPACE)) {
        audio_->PlayWave(bgmData_);
    }

    camera_->Update();

    #ifdef USE_IMGUI
    if (imguiManager_) {
        // スプライトの座標を操作するウィンドウを表示
        Vector2 pos = sprite1_->GetPosition();

        ImGui::SetNextWindowSize(ImVec2(500, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Sprite Control");
        ImGui::SliderFloat2("Position", &pos.x, 0.0f, 1280.0f, "%.1f");
        ImGui::End();

        sprite1_->SetPosition(pos);
    }
#endif

    sprite1_->Update();
}

void GamePlayScene::Draw()
{
    // スプライト描画
    spriteCommon_->CommonDrawSettings();
    sprite1_->Draw();
}

void GamePlayScene::Finalize()
{
    delete sprite1_;
    delete camera_;
    delete spriteCommon_;
}