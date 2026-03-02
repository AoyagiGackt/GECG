/**
 * @file SceneManager.h
 * @brief シーンの切り替え、更新、描画を一括管理するクラスを定義するファイル
 */
#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "AbstractSceneFactory.h"
#include <memory>

/**
 * @brief シーン運用を統括するマネージャークラス
 * @note シングルトンパターンを採用しており、どこからでも ChangeScene() を呼び出して
 * シーン遷移を予約できます。実際の切り替えは Update() の冒頭で安全に行われます
 */
class SceneManager {
public:
    
    /**
     * @brief SceneManagerの唯一のインスタンスを取得する
     * @return SceneManager* シングルトンインスタンスへのポインタ
     */
    static SceneManager* GetInstance();

    /**
     * @brief シーンマネージャーの初期化
     * @param dxCommon DirectX基盤のポインタ
     * @param input 入力管理のポインタ
     * @param audio 音響管理のポインタ
     * @param imgui ImGui管理のポインタ
     * @note 各シーンで共有する基盤システムのポインタを保持します
     */
    void Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio, ImGuiManager* imgui);
    
    /**
     * @brief 終了処理
     * @note 実行中のシーンを破棄し、後片付けを行います
     */
    void Finalize();

    /**
     * @brief シーンの更新処理
     * @note 次のシーンが予約されている場合は切り替え処理を行い、
     * その後現在アクティブなシーンの Update() を呼び出します
     */
    void Update();

    /**
     * @brief シーンの描画処理
     * @note 現在アクティブなシーンの Draw() を呼び出します
     */
    void Draw();

    /**
     * @brief 次のシーンを予約する
     * @param sceneName 生成したいシーンの名前（例: "TITLE", "GAMEPLAY"）
     * @note この関数を呼ぶと、次フレームの Update() の冒頭でシーンが切り替わります
     */
    void ChangeScene(const std::string& sceneName);
    
    /**
     * @brief シーン生成用工場をセットする
     * @param factory AbstractSceneFactoryを継承した具体的な工場のポインタ
     * @note シーン名から実際のクラスを生成するために必要です
     */
    void SetSceneFactory(AbstractSceneFactory* factory) { sceneFactory_ = factory; }


private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    const SceneManager& operator=(const SceneManager&) = delete;

    // --- 外部から提供される基盤システム ---
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    Audio* audio_ = nullptr;
    ImGuiManager* imguiManager_ = nullptr;
    
    // --- シーン管理メンバ ---

    /** @brief 現在アクティブなシーン */
    std::unique_ptr<BaseScene> currentScene_;

    /** @brief 次のフレームで切り替える予定のシーン */
    std::unique_ptr<BaseScene> nextScene_;

    /** @brief シーンを生成するための工場ポインタ（外部からセットされる） */
    AbstractSceneFactory* sceneFactory_ = nullptr;
};