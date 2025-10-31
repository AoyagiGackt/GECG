#pragma once
#include <Windows.h>
#include <cstdint>

// WindowsAPI
class WinApp {
public: // 静的メンバ関数
    // ウィンドウプロシージャ
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public: // メンバ関数
    // 初期化
    void Initialize();
    // 更新
    void Update();
    // 終了
    void Finalize();
    
    // getter
    HWND GetHwnd() const { return hwnd; }
    HINSTANCE GetHInstance() const { return wc.hInstance; }

public: // 定数
    // クライアント領域のサイズ
    static const int32_t kClientWidth = 1280;
    static const int32_t kClientHeight = 720;

private:
    // ウィンドウハンドル
    HWND hwnd = nullptr;
    // ウィンドウクラス
    WNDCLASS wc = {};
    // WindowsAPI
    WinApp* winApp_ = nullptr;
};