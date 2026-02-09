#pragma once
#include "Audio.h"
#include "DirectXCommon.h"
#include "Input.h"

// シーン名の定義
enum SceneType {
    kTitle,
    kGamePlay
};

// シーンの基底クラス
class BaseScene {
public:
    virtual ~BaseScene() = default;

    // 初期化
    virtual void Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio) = 0;

    // 更新
    virtual void Update() = 0;

    // 描画
    virtual void Draw() = 0;

    // 終了処理
    virtual void Finalize() = 0;

    virtual bool IsFinished() const { return false; }
};