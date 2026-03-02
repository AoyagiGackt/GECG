/**
 * @file ModelCommon.h
 * @brief 3Dモデルを描画するための共通グラフィックスパイプライン（PSO）やルートシグネチャを管理するファイル
 */
#pragma once
#include "DirectXCommon.h"
#include <wrl/client.h>

/**
 * @brief 3Dモデル描画の共通設定を保持するクラス
 * @note 各 Model インスタンスが個別にパイプラインを持つのではなく、
 * このクラスで一度だけ生成した設定（PSO等）を全モデルで共有することで、メモリと処理負荷を節約
 */
class ModelCommon {
public:

    /**
     * @brief 共通描画設定の初期化。ルートシグネチャとパイプラインステートオブジェクト(PSO)を生成する
     * @param dxCommon DirectX基盤のポインタ（デバイスの取得などに使用）
     */
    void Initialize(DirectXCommon* dxCommon);
    
    /**
     * @brief 描画コマンドにこの共通設定（ルートシグネチャとPSO）をセットする
     * @note 実際のモデルの描画（Model::Draw）を呼ぶ前に、毎フレーム必ず実行すること
     */
    void CommonDrawSettings();

    /**
     * @brief 保持しているDirectX共通クラスのポインタを取得する
     * @return DirectXCommon* DirectX基盤へのポインタ
     */
    DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:

    /** @brief DirectX基盤のポインタ（コマンドリストやデバイスの参照用） */
    DirectXCommon* dxCommon_;

    /** @brief シェーダーとリソースの紐づけルールを定義するルートシグネチャ */
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    /** @brief ブレンドや深度テストなどのグラフィックスパイプライン設定（PSO） */
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};