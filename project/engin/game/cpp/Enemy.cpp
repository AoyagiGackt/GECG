#include "Enemy.h"
#include "Player.h"

void Enemy::Initialize(ModelCommon* modelCommon, Player* player, const Vector3& startPos)
{
    player_ = player;

    // 3Dオブジェクトの初期化
    object3d_ = std::make_unique<Object3d>();
    object3d_->Initialize(modelCommon);
    object3d_->SetModel("Resources/enemy.obj");
    object3d_->GetTransform().translate = startPos;

    // AIの初期化
    ai_ = std::make_unique<EnemyAI>();
    ai_->Initialize(startPos);
}

void Enemy::Update()
{
    // プレイヤーの現在地をAIに伝えて思考させる
    ai_->Update(object3d_->GetTransform().translate, player_->GetTranslate());

    // AIが決めた移動量を座標に反映
    Vector3 velocity = ai_->GetVelocity();
    object3d_->GetTransform().translate.x += velocity.x;
    object3d_->GetTransform().translate.y += velocity.y;
    object3d_->GetTransform().translate.z += velocity.z;

    // 行列更新
    object3d_->Update();
}

void Enemy::Draw()
{
    object3d_->Draw();
}