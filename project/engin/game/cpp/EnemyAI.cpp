#include "EnemyAI.h"
#include <cmath>

/**
 * @brief 初期化
 */
void EnemyAI::Initialize(const Vector3& startPos)
{
    homePos_ = startPos;
    state_ = AIState::kIdle;
    velocity_ = { 0.0f, 0.0f, 0.0f };
}

/**
 * @brief 思考更新
 */
void EnemyAI::Update(const Vector3& selfPos, const Vector3& targetPos)
{
    // ターゲットとの距離を計算（三平方の定理）
    float dx = targetPos.x - selfPos.x;
    float dy = targetPos.y - selfPos.y;
    float dz = targetPos.z - selfPos.z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    // 距離に基づいた状態遷移ロジック
    if (distance < attackRange_) {
        state_ = AIState::kAttack;
    } else if (distance < detectionRange_) {
        state_ = AIState::kChase;
    } else {
        state_ = AIState::kPatrol;
    }

    // 決定された状態に応じた行動を計算
    switch (state_) {
    case AIState::kIdle: {
        ThinkIdle();
        break;
    }

    case AIState::kPatrol: {
        ThinkPatrol(selfPos);
        break;
    }

    case AIState::kChase: {
        ThinkChase(selfPos, targetPos);
        break;
    }

    case AIState::kAttack: {
        ThinkAttack();
        break;
    }

    case AIState::kEscape: {
        ThinkEscape(selfPos, targetPos);
        break;
    }
    }
}

void EnemyAI::ThinkIdle()
{
    velocity_ = { 0.0f, 0.0f, 0.0f };
}

void EnemyAI::ThinkPatrol(const Vector3& self)
{
    // 拠点に戻るようなゆるやかな動き
    Vector3 toHome = { homePos_.x - self.x, homePos_.y - self.y, homePos_.z - self.z };
    float mag = std::sqrt(toHome.x * toHome.x + toHome.y * toHome.y + toHome.z * toHome.z);
    if (mag > 0.1f) {
        velocity_ = { (toHome.x / mag) * (speed_ * 0.5f), 0.0f, (toHome.z / mag) * (speed_ * 0.5f) };
    } else {
        velocity_ = { 0.0f, 0.0f, 0.0f };
    }
}

void EnemyAI::ThinkChase(const Vector3& self, const Vector3& target)
{
    // ターゲットへ最短距離で向かうベクトル
    Vector3 dir = { target.x - self.x, target.y - self.y, target.z - self.z };
    float mag = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (mag > 0) {
        velocity_ = { (dir.x / mag) * speed_, (dir.y / mag) * speed_, (dir.z / mag) * speed_ };
    }
}

void EnemyAI::ThinkAttack()
{
    // 攻撃中は足を止める
    velocity_ = { 0.0f, 0.0f, 0.0f };
}

void EnemyAI::ThinkEscape(const Vector3& self, const Vector3& target)
{
    // ターゲットから遠ざかる方向ベクトル
    Vector3 dir = { self.x - target.x, self.y - target.y, self.z - target.z };
    float mag = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (mag > 0) {
        velocity_ = { (dir.x / mag) * speed_, (dir.y / mag) * speed_, (dir.z / mag) * speed_ };
    }
}

std::string EnemyAI::GetStateString() const
{
    switch (state_) {
    case AIState::kIdle: {
        return "Idle";
    }

    case AIState::kPatrol: {
        return "Patrol";
    }

    case AIState::kChase: {
        return "Chase";
    }

    case AIState::kAttack: {
        return "Attack";
    }

    case AIState::kEscape: {
        return "Escape";
    }
    }
    return "Unknown";
}