#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio)
{
    dxCommon_ = dxCommon;
    input_ = input;
    audio_ = audio;

    // スプライト初期化
    spriteCommon_ = std::make_unique<SpriteCommon>();
    spriteCommon_->Initialize(dxCommon_);

    // タイトル画像の生成(とりあえず)
    titleSprite_ = std::make_unique<Sprite>();
    titleSprite_->Initialize(spriteCommon_.get(), "Resources/uvChecker.png");
    titleSprite_->SetPosition({ 0.0f, 0.0f });
    // ウィンドウサイズに合わせる
    titleSprite_->SetSize({ 1280.0f, 720.0f });

    finished_ = false;
}

void TitleScene::Update()
{
    // スペースキーが押されたら終了フラグを立てる
    if (input_->TriggerKey(DIK_SPACE)) {
        SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    titleSprite_->Update();
}

void TitleScene::Draw()
{
    // 描画前処理
    spriteCommon_->CommonDrawSettings();

    // タイトル画像の描画
    titleSprite_->Draw();
}

void TitleScene::Finalize()
{
    
}