#pragma once

#include <array>
#include <dinput.h>
#include <windows.h>
#include <wrl.h>
#include "WinApp.h"

class Input {
public: // メンバ関数
    // namespace省略
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    // 初期化
    void Initialize(WinApp* winApp);
    // 更新
    void Update();

    bool PushKey(BYTE keyNumber);
    bool TriggerKey(BYTE keyNumber);

private:
    ComPtr<IDirectInput8> directInput_;
    ComPtr<IDirectInputDevice8> keyboard_;

    HINSTANCE hInstance_ = nullptr;
    HWND hwnd_ = nullptr;
    BYTE key[256] = {};
    BYTE keyPre[256] = {};

    std::array<BYTE, 256> keyStates_ {};
    std::array<BYTE, 256> prevKeyStates_ {};

    WinApp* winApp_ = nullptr;
};