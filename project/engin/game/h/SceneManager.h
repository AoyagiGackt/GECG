#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "AbstractSceneFactory.h"
#include <memory>

class SceneManager {
public:
    // どこからでもシーンを変えられるように
    static SceneManager* GetInstance();

    // 初期化・更新・描画
    void Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio, ImGuiManager* imgui);
    void Finalize();
    void Update();
    void Draw();

    // 次のシーンを予約する関数
    void ChangeScene(const std::string& sceneName);
    // 工場をセットする関数を追加
    void SetSceneFactory(AbstractSceneFactory* factory) { sceneFactory_ = factory; }


private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    const SceneManager& operator=(const SceneManager&) = delete;

    // 必要なもの
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    Audio* audio_ = nullptr;

    ImGuiManager* imguiManager_ = nullptr;
    // 今のシーン
    std::unique_ptr<BaseScene> currentScene_;
    // 次のシーン（予約）
    std::unique_ptr<BaseScene> nextScene_;
    AbstractSceneFactory* sceneFactory_ = nullptr;
};