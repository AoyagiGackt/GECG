#include "Input.h"

void Input::Initialize() {
    // DirectInput�̃C���X�^���X����
    ComPtr<IDirectInput8> directInput = nullptr;
    result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)SdirectInput, nullptr);
    assert(SUCCEEDED(result));
    // �L�[�{�[�h�f�o�C�X����
    ComPtr<IDirectInputDevice8> keyboard;
    result = directInput->CreateDevice(GUID_SysKeyboard, keyboard, NULL);
    assert(SUCCEEDED(result));
    // ���̓f�[�^�`���̃Z�b�g
    result = keyboard->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(result));
    // �r�����䃌�x���̃Z�b�g
    result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));
}

void Input::Update() {
}