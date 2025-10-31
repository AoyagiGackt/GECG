#pragma once

#include <array>
#include <dinput.h>
#include <windows.h>
#include <wrl.h>

class Input {
public: // メンバ関数
    // namespace省略
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

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