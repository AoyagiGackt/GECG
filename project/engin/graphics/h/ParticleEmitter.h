#pragma once
#include "MakeAffine.h"
#include "ParticleManager.h"

class ParticleEmitter {
public:
    ParticleEmitter(ParticleManager* particleManager);

    // 更新
    void Update();

    // 発生場所のセット
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

private:
    ParticleManager* particleManager_ = nullptr;
    Transform transform_;

    // 発生間隔など
    float count_ = 0.0f;
    float frequency_ = 0.1f; // 秒
};