#pragma once

#include <windows.h>
#include <dinput.h>
#include <wrl.h>
#include <array>

using namespace Microsoft::WRL;

class Input {
public: // メンバ関数
    // 初期化
    void Initialize(HINSTANCE hInstance, HWND hwnd);
    // 更新
    void Update();

private:
    ComPtr<IDirectInput8> directInput_;
    ComPtr<IDirectInputDevice8> keyboard_;

    HINSTANCE hInstance_ = nullptr;
    HWND hwnd_ = nullptr;

    std::array<BYTE, 256> keyStates_ {};
    std::array<BYTE, 256> prevKeyStates_ {};
};