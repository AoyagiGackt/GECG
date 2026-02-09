#pragma once
#include "Camera.h"
#include "Framework.h"
#include "ModelCommon.h"
#include "Object3dCommon.h"
#include "Sprite.h"
#include "SpriteCommon.h"

class MyGame : public Framework {
public:
    // 初期化
    void Initialize() override;
    // 解放
    void Finalize() override;
    // 更新処理
    void Update() override;
    // 描画処理
    void Draw() override;

private:
    // 描画共通
    SpriteCommon* spriteCommon_ = nullptr;
    ModelCommon* modelCommon_ = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;

    // オブジェクト
    Sprite* sprite1_ = nullptr;
    Camera* camera_ = nullptr;

    // サウンドデータ
    SoundData bgmData_;
};