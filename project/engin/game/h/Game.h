#pragma once
#include <memory>

// 基盤系
#include "Audio.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "WinApp.h"

// 描画共通
#include "ModelCommon.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"

// シーンオブジェクト
#include "Camera.h"
#include "Object3d.h"
#include "Sprite.h"

// ライブラリのリンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

class Game {
public:
    void Initialize();
    void Update();
    void Draw();
    void Finalize();

    // 終了フラグの取得
    bool IsEndRequest() const { return endRequest_; }

private:
    // ゲームの基盤
    WinApp* winApp_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    Audio* audio_ = nullptr;
    ImGuiManager* imguiManager_ = nullptr;

    // 描画設定・共通
    SpriteCommon* spriteCommon_ = nullptr;
    ModelCommon* modelCommon_ = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;

    // シーン用ポインタ
    Camera* camera_ = nullptr;
    Sprite* sprite1_ = nullptr;

    // サウンドデータ
    SoundData bgmData_;

    bool endRequest_ = false;
};