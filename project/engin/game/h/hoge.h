#pragma once

// エンジンの基本機能をインクルード
#include "Audio.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "GameObject.h"

/// <summary>
/// Hogeクラス
/// </summary>
class Hoge : public GameObject {
public:
    // コンストラクタ
    Hoge();
    // デストラクタ
    ~Hoge();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="dxCommon">DirectX共通機能</param>
    /// <param name="input">入力機能</param>
    /// <param name="audio">音響機能</param>
    void Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio);

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

private:
    // --- 借りてくるもの ---
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    Audio* audio_ = nullptr;

    // --- メンバ変数 ---
};