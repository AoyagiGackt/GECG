#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定

#include "Input.h"
#include <cassert>
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(WinApp* winApp)
{
    HRESULT result;
    
    // 借りてきたWinAppのインスタンスを記録
    this->winApp_ = winApp;

    // DirectInputのインスタンス生成
    result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
    assert(SUCCEEDED(result));
    // キーボードデバイス生成
    result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
    assert(SUCCEEDED(result));
    // 入力データ形式のセット
    result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(result));
    // 排他制御レベルのセット
    result = keyboard_->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));
}

void Input::Update() {
    HRESULT result;

    // 前回のキー入力を保存
    memcpy(keyPre, key, sizeof(key));
    // キーボード情報の取得開始
    result = keyboard_->Acquire();
    // 全キーの入力情報を取得する
    result = keyboard_->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber)
{
    // 指定キーを押していればtrueを返す
    if (key[keyNumber]) {
        return true;
    }

    // そうでなければfalseを返す
    return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
    // 前回は押してなくて今回は押している
    if (key[keyNumber] && !prevKeyStates_[keyNumber]) {
        return true;
    }
    // そうでなければfalseを返す
    prevKeyStates_[keyNumber] = key[keyNumber];
    return false;
}