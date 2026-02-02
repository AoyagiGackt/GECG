#pragma once
#include "DirectXCommon.h"
#include "WinApp.h"

#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#endif

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