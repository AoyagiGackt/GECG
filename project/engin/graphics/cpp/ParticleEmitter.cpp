#include "ParticleEmitter.h"
#include <cstdlib>

ParticleEmitter::ParticleEmitter(ParticleManager* particleManager)
    : particleManager_(particleManager)
{
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, 0.0f };
}

void ParticleEmitter::Update()
{
    count_ += 1.0f / 60.0f;

    if (count_ >= frequency_) {
        count_ = 0.0f;

        Vector3 velocity;
        velocity.x = (float)(rand() % 100 - 50) / 100.0f;
        velocity.y = (float)(rand() % 100 - 50) / 100.0f;
        velocity.z = (float)(rand() % 100 - 50) / 100.0f;

        particleManager_->Emit("default", transform_.translate, velocity);
    }
}