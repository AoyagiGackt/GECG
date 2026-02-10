#include "Game.h"
#include "GamePlayScene.h"
#include "SceneManager.h"
#include "SceneFactory.h"
#include "TitleScene.h"
#include <SrvManager.h>

void MyGame::Initialize()
{
    // 基盤の初期化
    Framework::Initialize();

    // 工場を作る
    sceneFactory_ = std::make_unique<SceneFactory>();

    // SceneManagerに工場を教える
    SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());

    // 最初のシーンを工場経由でセットする
    SceneManager::GetInstance()->Initialize(
        dxCommon_.get(),
        input_.get(),
        audio_.get(),
        imguiManager_.get());
}


void MyGame::Update()
{
    // 基盤の更新
    Framework::Update();

    // シーンマネージャー更新
    SceneManager::GetInstance()->Update();

    // ImGui終了処理
    imguiManager_->End();
}

void MyGame::Draw()
{
    dxCommon_->PreDraw();
    SrvManager::GetInstance()->PreDraw();

    // 現在のシーンの描画
    SceneManager::GetInstance()->Draw();

    imguiManager_->Draw(dxCommon_.get());

    dxCommon_->PostDraw();
}

void MyGame::Finalize()
{
    audio_->Finalize();

    SceneManager::GetInstance()->Finalize();

    Framework::Finalize();
}