#pragma once
#include "DirectXCommon.h"
#include "WinApp.h"

class ImGuiManager {
public:
    // 初期化
    void Initialize(WinApp* winApp, DirectXCommon* dxCommon);

    // 開始
    void Begin();

    // 描画
    void End();

    // 描画
    void Draw(DirectXCommon* dxCommon);

    // 終了処理
    void Finalize();
};