/**
 * @file Object3dCommon.h
 * @brief 3Dオブジェクトの描画に必要な共通設定（主にデフォルトのライティング）を管理するファイル
 */
#pragma once
#include "DirectXCommon.h"

/**
 * @brief 3Dオブジェクト全体で共有される設定やリソース（ライトなど）を管理するクラス
 * @note 各 Object3d インスタンスを描画する前に、このクラスで管理しているライト情報を
 * コマンドリストにセットすることで、モデルに影をつけることができます。
 */
class Object3dCommon {
public:

    /**
     * @brief 共通設定の初期化。デフォルトのディレクショナルライト（平行光源）用定数バッファを作成する
     * @param dxCommon DirectX基盤のポインタ（デバイス取得などに使用）
     */
    void Initialize(DirectXCommon* dxCommon);

    /**
     * @brief 描画コマンドリストにデフォルトライトの定数バッファをセットする
     * @param commandList 描画コマンドを積むためのグラフィックスコマンドリスト
     * @note 実際のモデルの描画（Object3d::Draw 等）を行う前に、毎フレーム実行すること。
     */
    void SetDefaultLight(ID3D12GraphicsCommandList* commandList);

private:

    /** @brief DirectX基盤のポインタ（デバイスやコマンドリストの参照用） */
    DirectXCommon* dxCommon_ = nullptr;

    /** @brief デフォルトライトのデータ（色、方向、強度など）を保持するGPUリソース（定数バッファ） */
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultLightResource_;
};