#include "SceneManager.h"
#include "GamePlayScene.h"
#include "TitleScene.h"

SceneManager* SceneManager::GetInstance()
{
    static SceneManager instance;
    return &instance;
}

void SceneManager::Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio, ImGuiManager* imgui)
{
    dxCommon_ = dxCommon;
    input_ = input;
    audio_ = audio;
    imguiManager_ = imgui;

    // 最初のシーン
    currentScene_ = new TitleScene();
    currentScene_->Initialize(dxCommon_, input_, audio_);

    auto titleScene = dynamic_cast<TitleScene*>(currentScene_);
    if (titleScene) {
        titleScene->SetImGuiManager(imguiManager_);
    }
}

void SceneManager::Update()
{
    // 次のシーン予約があれば、切り替える
    if (nextScene_) {
        // 旧シーン終了
        if (currentScene_) {
            currentScene_->Finalize();
            delete currentScene_;
        }
        // 新シーンへ交代
        currentScene_ = nextScene_;
        nextScene_ = nullptr; // 予約クリア

        // 新シーン初期化
        currentScene_->Initialize(dxCommon_, input_, audio_);

        auto gameplayScene = dynamic_cast<GamePlayScene*>(currentScene_);
        if (gameplayScene) {
            gameplayScene->SetImGuiManager(imguiManager_);
        }
    }

    // 今のシーンの更新
    if (currentScene_) {
        currentScene_->Update();
    }
}

void SceneManager::Draw()
{
    if (currentScene_) {
        currentScene_->Draw();
    }
}

void SceneManager::Finalize()
{
    if (currentScene_) {
        currentScene_->Finalize();
        delete currentScene_;
    }
}

// シーン切り替え予約
void SceneManager::ChangeScene(const std::string& sceneName)
{
    if (sceneName == "TITLE") {
        nextScene_ = new TitleScene();
    } else if (sceneName == "GAMEPLAY") {
        nextScene_ = new GamePlayScene();
    }
}