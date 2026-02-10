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
    currentScene_ = std::make_unique<TitleScene>();
    currentScene_->Initialize(dxCommon_, input_, audio_);

   auto titleScene = dynamic_cast<TitleScene*>(currentScene_.get());
    if (titleScene) {
        titleScene->SetImGuiManager(imguiManager_);
    }
}

void SceneManager::Update()
{
    // 次のシーン予約があれば、切り替える
    if (nextScene_) {
        currentScene_ = std::move(nextScene_);

        // 初期化
        currentScene_->Initialize(dxCommon_, input_, audio_);

        // ImGuiのセット
        auto gameplayScene = dynamic_cast<GamePlayScene*>(currentScene_.get());
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
    }

    currentScene_.reset();
    nextScene_.reset();
}

// シーン切り替え予約
void SceneManager::ChangeScene(const std::string& sceneName)
{
    // 工場にシーンを作ってもらう
    nextScene_ = sceneFactory_->CreateScene(sceneName);
}