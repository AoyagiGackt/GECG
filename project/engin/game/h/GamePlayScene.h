#pragma once
#include "Audio.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "BaseScene.h"
#include "hoge.h"
#include "Input.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include "GameObject.h"
#include <memory>
#include <vector>

class GamePlayScene : public BaseScene {
public:
    // 初期化時に必要なマネージャーを受け取る
    void Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio) override;
    void Finalize() override;
    void Update() override;
    void Draw() override;

    void SetImGuiManager(ImGuiManager* imgui) { imguiManager_ = imgui; }

private:
    // 借りてくるもの
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    Audio* audio_ = nullptr;
    ImGuiManager* imguiManager_ = nullptr;

    // このシーンで管理するもの
    std::unique_ptr<SpriteCommon> spriteCommon_;
    std::unique_ptr<Sprite> sprite1_;
    std::unique_ptr<Camera> camera_;
    std::vector<std::unique_ptr<GameObject>> gameObjects_;
    SoundData bgmData_;
};