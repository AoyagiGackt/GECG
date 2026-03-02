#pragma once
#include "LightingMode.h"

class LightManager {
public:
    // シングルトンインスタンスの取得
    static LightManager* GetInstance()
    {
        static LightManager instance;
        return &instance;
    }

    // ゲッターとセッター
    void SetLightingMode(int mode) { lightingMode_ = mode; }
    int GetLightingMode() const { return lightingMode_; }

private:
    LightManager() = default;
    ~LightManager() = default;
    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    // ここで状態を管理する
    int lightingMode_ = LightingMode::Lighting_HalfLambert;
};