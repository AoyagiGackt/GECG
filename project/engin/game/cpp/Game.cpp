#include "Game.h"
#include "GamePlayScene.h"
#include <SrvManager.h>

void MyGame::Initialize()
{
    // 基盤の初期化
    Framework::Initialize();

    // 最初のシーンとしてゲームプレイシーンを生成
    scene_ = new GamePlayScene();

    // シーンの初期化
    scene_->Initialize(dxCommon_, input_, audio_);

    auto gameplayScene = dynamic_cast<GamePlayScene*>(scene_);
    if (gameplayScene) {
        gameplayScene->SetImGuiManager(imguiManager_);
    }
}

void MyGame::Update()
{
    // 基盤の更新
    Framework::Update();

    // 現在のシーンの更新
    scene_->Update();

    // ImGui終了処理
    imguiManager_->End();
}

void MyGame::Draw()
{
    dxCommon_->PreDraw();
    SrvManager::GetInstance()->PreDraw();

    // 現在のシーンの描画
    scene_->Draw();

    imguiManager_->Draw(dxCommon_);
    dxCommon_->PostDraw();
}

void MyGame::Finalize()
{
    audio_->Finalize();

    // シーンの終了処理
    if (scene_) {
        scene_->Finalize();

        delete scene_;
        scene_ = nullptr;
    }

    // 基盤の終了
    Framework::Finalize();
}