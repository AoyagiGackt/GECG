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
    spriteCommon_ = std::make_unique<SpriteCommon>();
    spriteCommon_->Initialize(dxCommon_);

    // アセットロード
    bgmData_ = audio_->LoadAudio("Resources/461_BPM174.wav");

    // オブジェクト生成
    camera_ = std::make_unique<Camera>(); // カメラ生成
    sprite1_ = std::make_unique<Sprite>();
    sprite1_->Initialize(spriteCommon_.get(), "Resources/uvChecker.png");
    sprite1_->SetPosition({ 100.0f, 100.0f });

    auto hoge = std::make_unique<Hoge>();
    hoge->Initialize(dxCommon, input, audio);
    gameObjects_.push_back(std::move(hoge));
}

void GamePlayScene::Update()
{
    // ゲームロジック
    if (input_->TriggerKey(DIK_SPACE)) {
        audio_->PlayWave(bgmData_);
    }

    camera_->Update();
    
    for (auto& obj : gameObjects_) {
        obj->Update();
    }

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
    
    for (auto& obj : gameObjects_) {
        obj->Draw();
    }
}

void GamePlayScene::Finalize()
{

}