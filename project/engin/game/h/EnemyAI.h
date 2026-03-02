/**
 * @file EnemyAI.h
 * @brief 敵キャラクターの意思決定（AI）ロジックを管理するファイル
 */
#pragma once
#include "MakeAffine.h"
#include <string>

/**
 * @brief AIの状態を定義する列挙型
 */
enum class AIState {
    kIdle, ///< 待機：その場で停止
    kPatrol, ///< 巡回：決まったルートを移動
    kChase, ///< 追跡：ターゲット（プレイヤー）を追いかける
    kAttack, ///< 攻撃：射程内に入ったターゲットを攻撃
    kEscape ///< 撤退：体力が減った際などに距離を取る
};

/**
 * @brief 敵キャラクターの知能（CPU）を制御するクラス
 */
class EnemyAI {
public:
    /**
     * @brief AIの初期化
     * @param startPos 初期配置座標
     */
    void Initialize(const Vector3& startPos);

    /**
     * @brief AIの思考更新。毎フレーム呼び出すことで状態を判定し、行動を決定する
     * @param selfPos 自身の現在位置
     * @param targetPos ターゲット（プレイヤーなど）の現在位置
     */
    void Update(const Vector3& selfPos, const Vector3& targetPos);

    /**
     * @brief AIが決定した現在の移動ベクトルを取得する
     * @return const Vector3& 移動方向と速さ
     */
    const Vector3& GetVelocity() const { return velocity_; }

    /**
     * @brief 現在の状態を文字列で取得する（デバッグ表示用）
     * @return std::string 状態名
     */
    std::string GetStateString() const;

private:
    /** @brief 各状態における具体的な計算 */
    void ThinkIdle();
    void ThinkPatrol(const Vector3& self);
    void ThinkChase(const Vector3& self, const Vector3& target);
    void ThinkAttack();
    void ThinkEscape(const Vector3& self, const Vector3& target);

private:
    AIState state_ = AIState::kIdle; ///< 現在の状態
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; ///< 決定された移動ベクトル
    Vector3 homePos_ = { 0.0f, 0.0f, 0.0f }; ///< 拠点（戻るべき場所）の座標

    // --- パラメータ（ImGui等で調整可能にすると便利） ---
    float speed_ = 0.08f; ///< 移動速度
    float detectionRange_ = 15.0f; ///< 索敵範囲（追跡に切り替わる距離）
    float attackRange_ = 3.0f; ///< 攻撃範囲
};