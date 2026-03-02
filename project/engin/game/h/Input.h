/**
 * @file Input.h
 * @brief DirectInput 8 を使用したキーボード入力管理を行うファイル
 */
#pragma once

#include <array>
#include <dinput.h>
#include <windows.h>

#include <wrl/client.h>
#include "WinApp.h"

/**
 * @brief キーボード入力を一括管理するクラス
 * @note DirectInput 8 を使用して、全256キーの状態を取得・管理します
 * 押しっぱなし判定（PushKey）と、押した瞬間判定（TriggerKey）を提供します
 */
class Input {
public: // メンバ関数
    // namespace省略
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    /**
     * @brief 入力システムの初期化
     * @param winApp ウィンドウ管理クラスのポインタ
     * @note DirectInput インスタンスの生成、キーボードデバイスの初期化、データ形式の設定などを行います
     */
    void Initialize(WinApp* winApp);

    /**
     * @brief キー状態の更新
     * @note 毎フレーム呼び出すことで、現在のキー状態を「前回の状態」にコピーし、
     * デバイスから最新のキー状態を取得して更新します
     */
    void Update();

    /**
     * @brief キーが押されている（押しっぱなし）かチェック
     * @param keyNumber キーの番号（例: DIK_SPACE）
     * @return bool 押されていれば true
     */
    bool PushKey(BYTE keyNumber);
    
    /**
     * @brief キーが押された瞬間かチェック
     * @param keyNumber キーの番号（例: DIK_RETURN）
     * @return bool このフレームで押された瞬間なら true
     * @note 1フレーム前が「離されていた」かつ「現在が押されている」場合に true を返します
     */
    bool TriggerKey(BYTE keyNumber);

private:

    /** @brief DirectInput 8 の本体ポインタ */
    ComPtr<IDirectInput8> directInput_;

    /** @brief キーボードデバイスのポインタ */
    ComPtr<IDirectInputDevice8> keyboard_;

    /** @brief インスタンスハンドル */
    HINSTANCE hInstance_ = nullptr;

    /** @brief ウィンドウハンドル */
    HWND hwnd_ = nullptr;

    // --- キー状態管理用バッファ ---

    /** @brief 最新のキー状態（256個のキー分） */
    BYTE key[256] = {};

    /** @brief 1フレーム前のキー状態（256個のキー分） */
    BYTE keyPre[256] = {};

    /** @brief 最新のキー状態（std::array版） */
    std::array<BYTE, 256> keyStates_ {};

    /** @brief 1フレーム前のキー状態（std::array版） */
    std::array<BYTE, 256> prevKeyStates_ {};

    /** @brief ウィンドウ管理のポインタ */
    WinApp* winApp_ = nullptr;
};