/**
 * @file Player.h
 * @brief プレイヤーキャラクターの操作と状態を管理するファイル
 */
#pragma once
#include "GameObject.h"
#include "Input.h"
#include "Object3d.h"

/**
 * @brief プレイヤーキャラクタークラス
 * @note GameObjectを継承し、キーボード入力による移動と3Dモデルの描画を行います
 */
class Player : public GameObject {
public:
    /**
     * @brief 初期化処理
     * @param modelCommon 3Dモデル共通設定のポインタ
     * @param input 入力管理クラスのポインタ
     */
    void Initialize(ModelCommon* modelCommon, Input* input);

    /**
     * @brief 更新処理
     * @note 入力に応じた移動計算と、3Dオブジェクトの行列更新を行います
     */
    void Update() override;

    /**
     * @brief 描画処理
     */
    void Draw() override;

    /** @brief 座標の取得 */
    const Vector3& GetTranslate() const { return object3d_->GetTransform().translate; }

private:
    /** @brief 操作用の入力クラスへのポインタ */
    Input* input_ = nullptr;

    /** @brief 描画用の3Dオブジェクト */
    std::unique_ptr<Object3d> object3d_;

    /** @brief 移動速度 */
    float speed_ = 0.1f;
};