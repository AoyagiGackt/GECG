#pragma once
#include "Framework.h"
#include "BaseScene.h"
#include "SceneFactory.h"
#include <memory>

class MyGame : public Framework {
public:
    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;

private:
    BaseScene* scene_ = nullptr;
    std::unique_ptr<AbstractSceneFactory> sceneFactory_;
};