#pragma once
#include "MakeAffine.h"
#include "ParticleManager.h"

class ParticleEmitter {
public:
   ParticleEmitter(const std::string& name, const Transform& transform);

    // 更新
    void Update();

    void Emit();

    // 発生場所のセット
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

private:
    std::string name_;
    Transform transform_;

    // 発生間隔など
    float frequency_ = 0.5f;
    float timeCount_ = 0.0f;
};