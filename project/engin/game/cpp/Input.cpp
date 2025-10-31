#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定

#include "Input.h"
#include <cassert>
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
    HRESULT result;

    // DirectInputのインスタンス生成
    ComPtr<IDirectInput8> directInput = nullptr;
    result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
    assert(SUCCEEDED(result));
    // キーボードデバイス生成
    result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
    assert(SUCCEEDED(result));
    // 入力データ形式のセット
    result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(result));
    // 排他制御レベルのセット
    result = keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));
}

void Input::Update() {
    // キーボード情報の取得開始
    keyboard_->Acquire();
    // 全キーの入力情報を取得する
    BYTE key[256] = {};
    keyboard_->GetDeviceState(sizeof(key), key);
}