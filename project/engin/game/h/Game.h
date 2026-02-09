#pragma once
#include "Framework.h"
#include "IScene.h"
#include <memory>

class MyGame : public Framework {
public:
    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;

private:
    IScene* scene_ = nullptr;
};