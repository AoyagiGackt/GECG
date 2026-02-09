#pragma once

// 標準ライブラリ
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

class Framework {
public:
    virtual ~Framework() = default;

    // 全体の流れを制御する
    void Run();

    // 子クラスで中身を書く関数
    virtual void Initialize();
    virtual void Finalize();
    virtual void Update();
    virtual void Draw() = 0; // 描画だけ個別に

    // 終了判定
    virtual bool IsEndRequest() { return endRequest_ || winApp_->ProcessMessage(); }

protected:
    WinApp* winApp_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    Audio* audio_ = nullptr;
    ImGuiManager* imguiManager_ = nullptr;
    bool endRequest_ = false;
};