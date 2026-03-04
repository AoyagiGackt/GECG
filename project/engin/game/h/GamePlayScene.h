/**
 * @file GamePlayScene.h
 * @brief ゲームプレイ本編のシーンロジックを管理するファイル
 */
#pragma once
#include "Audio.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "BaseScene.h"
#include "hoge.h"
#include "Input.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include "GameObject.h"
#include "VideoPlayer.h"
#include <memory>
#include <vector>

/**
 * @brief ゲームプレイ本編のシーンクラス
 * @note BaseScene を継承し、実際のゲーム進行（キャラクターの動か、スコア計算、描画等）を記述します。
 * 複数の GameObject をリストで管理し、ポリモーフィズムを利用して一括更新・描画を行います。
 */
class GamePlayScene : public BaseScene {
public:
    
    /**
     * @brief シーンの初期化
     * @param dxCommon DirectX基盤のポインタ
     * @param input 入力管理のポインタ
     * @param audio 音響管理のポインタ
     * @note スプライトやカメラの生成、BGMのロード、ゲームオブジェクトの配置などを行います
     */
    void Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio) override;
    
    /**
     * @brief シーンの終了処理
     * @note シーンで使用したリソースの解放や、音声の停止などを行います
     */
    void Finalize() override;

    /**
     * @brief シーンの更新処理
     * @note プレイヤーの操作、敵の挙動、衝突判定など、ゲームのメインロジックを毎フレーム更新します
     */
    void Update() override;

    /**
     * @brief シーンの描画処理
     * @note 3Dオブジェクト、背景スプライト、UIなどの描画コマンドを積み込みます
     */
    void Draw() override;

    /**
     * @brief デバッグ用UIマネージャーをセットする
     * @param imgui ImGuiManagerのポインタ
     */
    void SetImGuiManager(ImGuiManager* imgui) { imguiManager_ = imgui; }

private:
    
    // --- 外部から提供される基盤システム（借りてくるもの） ---

    /** @brief DirectX基盤のポインタ */
    DirectXCommon* dxCommon_ = nullptr;

    /** @brief 入力管理のポインタ */
    Input* input_ = nullptr;

    /** @brief 音響管理のポインタ */
    Audio* audio_ = nullptr;

    /** @brief デバッグUI用のImGuiマネージャー */
    ImGuiManager* imguiManager_ = nullptr;

    // --- このシーンが所有・管理するリソース ---

    /** @brief スプライト描画の共通設定 */
    std::unique_ptr<SpriteCommon> spriteCommon_;

    /** @brief 背景やUIに使用するサンプルスプライト */
    std::unique_ptr<Sprite> sprite1_;

    /** @brief シーンのメインカメラ */
    std::unique_ptr<Camera> camera_;

    /** * @brief 登録されているゲームオブジェクト（プレイヤー、敵など）のリスト
     * @note GameObject を継承したクラスを一括で管理し、Update/Draw を回します
     */
    std::vector<std::unique_ptr<GameObject>> gameObjects_;

    /** @brief このシーンで再生するBGMのデータ */
    SoundData bgmData_;

    /** @brief 動画再生用のVideoPlayer */
    std::unique_ptr<VideoPlayer> videoPlayer_;

    /** @brief 動画ファイルのパスリスト（複数動画を切り替える場合などに使用） */
    std::vector<std::string> videoList_;

    /** @brief 現在再生中の動画のインデックス */
    int currentVideoIndex_ = 0;
};