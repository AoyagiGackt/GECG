#pragma once
#include "Audio.h"
#include "DirectXCommon.h"
#include "BaseScene.h"
#include "Input.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <memory>

class TitleScene : public BaseScene {
public:
    void Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio) override;
    void Finalize() override;
    void Update() override;
    void Draw() override;

    // シーンが終了したかどうか
    bool IsFinished() const { return finished_; }
    void SetImGuiManager(ImGuiManager* imgui) { imguiManager_ = imgui; }

private:
    // 借りてくるもの
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    Audio* audio_ = nullptr;
    ImGuiManager* imguiManager_ = nullptr;

    // 自分の持ち物
    std::unique_ptr<SpriteCommon> spriteCommon_;
    std::unique_ptr<Sprite> titleSprite_; // タイトル画像用

    // 終了フラグ
    bool finished_ = false;
};
