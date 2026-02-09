#pragma once
#include "Audio.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "IScene.h"
#include "Input.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"

class GamePlayScene : public IScene {
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
    SpriteCommon* spriteCommon_ = nullptr;
    Sprite* sprite1_ = nullptr;
    Camera* camera_ = nullptr;
    SoundData bgmData_;
};