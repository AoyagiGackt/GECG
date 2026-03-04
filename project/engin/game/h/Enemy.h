/**
 * @file Enemy.h
 * @brief AIを搭載した敵キャラクターを管理するファイル
 */
#pragma once
#include "EnemyAI.h"
#include "GameObject.h"
#include "Object3d.h"

class Player; // 循環参照防止の前方宣言

/**
 * @brief 敵キャラクタークラス
 */
class Enemy : public GameObject {
public:
    /**
     * @brief 初期化処理
     * @param modelCommon 3Dモデル共通設定のポインタ
     * @param player 追跡対象となるプレイヤーのポインタ
     * @param startPos 初期出現座標
     */
    void Initialize(ModelCommon* modelCommon, Player* player, const Vector3& startPos);

    /**
     * @brief 更新処理
     */
    void Update() override;

    /**
     * @brief 描画処理
     */
    void Draw() override;

private:
    /** @brief 描画用の3Dオブジェクト */
    std::unique_ptr<Object3d> object3d_;

    /** @brief 思考を司るAIクラス */
    std::unique_ptr<EnemyAI> ai_;

    /** @brief 追跡対象のプレイヤー */
    Player* player_ = nullptr;
};