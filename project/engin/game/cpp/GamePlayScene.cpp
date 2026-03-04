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

    // 動画プレイヤーの生成
    videoPlayer_ = std::make_unique<VideoPlayer>();
    
    // 動画再生の初期化
    videoPlayer_->Initialize(dxCommon_, spriteCommon_.get(), "Resources/test.mp4");

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

        if (videoPlayer_) {
            ImGui::Begin("Video Player Settings");

            // 座標の変更
            Vector2 pos = videoPlayer_->GetPosition();
            if (ImGui::DragFloat2("Position", &pos.x, 1.0f)) {
                videoPlayer_->SetPosition(pos);
            }

            // サイズの変更
            Vector2 size = videoPlayer_->GetSize();
            if (ImGui::DragFloat2("Size", &size.x, 1.0f)) {
                videoPlayer_->SetSize(size);
            }

            // 回転の変更
            float rot = videoPlayer_->GetRotation();
            if (ImGui::DragFloat("Rotation", &rot, 0.01f)) {
                videoPlayer_->SetRotation(rot);
            }

            // 再生速度
            float duration = videoPlayer_->GetFrameDuration();
            if (ImGui::DragFloat("Frame Duration", &duration, 0.001f, 0.001f, 0.1f, "%.3f")) {
                videoPlayer_->SetFrameDuration(duration);
            }

            ImGui::End();
        }
    }
#endif

    sprite1_->Update();

    if (videoPlayer_) {
        videoPlayer_->Update();
    }
}

void GamePlayScene::Draw()
{
    // スプライト描画
    spriteCommon_->CommonDrawSettings();
    sprite1_->Draw();
    
    if (videoPlayer_) {
        videoPlayer_->Draw();
    }

    for (auto& obj : gameObjects_) {
        obj->Draw();
    }
}

void GamePlayScene::Finalize()
{

}