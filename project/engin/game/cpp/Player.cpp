#include "Player.h"

void Player::Initialize(ModelCommon* modelCommon, Input* input)
{
    input_ = input;

    // 3Dオブジェクトの生成と初期化
    object3d_ = std::make_unique<Object3d>();
    object3d_->Initialize(modelCommon);
    object3d_->SetModel("Resources/player.obj");
}

void Player::Update()
{
    // --- キーボード入力 ---
    Vector3 move = { 0, 0, 0 };
    if (input_->PushKey(DIK_W))
        move.z += 1.0f;
    if (input_->PushKey(DIK_S))
        move.z -= 1.0f;
    if (input_->PushKey(DIK_A))
        move.x -= 1.0f;
    if (input_->PushKey(DIK_D))
        move.x += 1.0f;

    // --- コントローラー入力 ---
    input_->UpdateGamepad(); // 毎フレーム呼ぶ
    Input::Stick stick = input_->GetLeftStick();

    // スティックの傾きを移動量に加算
    move.x += stick.x;
    move.z += stick.y;

    // 正規化して斜め移動が速くならないようにする
    float length = std::sqrt(move.x * move.x + move.z * move.z);
    if (length > 0) {
        move.x = (move.x / length) * speed_;
        move.z = (move.z / length) * speed_;
    }

    // 座標更新
    auto& transform = object3d_->GetTransform();
    transform.translate.x += move.x;
    transform.translate.z += move.z;

    // ボタンでジャンプ（例：Aボタン）
    if (input_->TriggerButton(XINPUT_GAMEPAD_A)) {
        // ジャンプ処理...
    }

    object3d_->Update();
}

void Player::Draw()
{
    object3d_->Draw();
}