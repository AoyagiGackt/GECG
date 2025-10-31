#pragma once
#include <Windows.h>

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
};