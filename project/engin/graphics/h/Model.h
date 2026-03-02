/**
 * @file Model.h
 * @brief 3Dモデル（OBJファイル等）の頂点データやテクスチャ情報を保持し、描画を行うファイル
 */
#pragma once
#include "DirectXCommon.h"
#include "MakeAffine.h"
#include <string>
#include <vector>

class ModelCommon;

/**
 * @brief 1つの3Dモデルデータを管理するクラス
 * @note 初期化時にOBJファイルを読み込み、GPUに転送するための頂点バッファを作成します。
 */
class Model {
public:

    /**
     * @brief モデルを構成する1つの頂点データ
     */
    struct VertexData {
        
        /** @brief 頂点座標（ローカル空間の X, Y, Z, W） */
        Vector4 position;

        /** @brief UV座標（テクスチャマッピング用の U, V） */
        Vector2 texcoord;

        /** @brief 頂点法線ベクトル（ライティングの陰影計算に使用） */
        Vector3 normal;
    };

    /**
     * @brief モデルの初期化（OBJファイルとテクスチャの読み込み、頂点バッファの生成）
     * @param modelCommon モデル描画の共通設定を持つオブジェクトのポインタ
     * @param modelFilePath 読み込むOBJファイルのパス（例: "Resources/model.obj"）
     * @param textureFilePath 貼り付けるテクスチャ画像のパス（例: "Resources/texture.png"）
     */
    void Initialize(ModelCommon* modelCommon, const std::string& modelFilePath, const std::string& textureFilePath);
    
    /**
     * @brief モデルの描画コマンド（DrawInstanced）を積む
     * @param modelCommon モデル描画の共通設定を持つオブジェクトのポインタ
     */
    void Draw(ModelCommon* modelCommon);

    /**
     * @brief このモデルの頂点バッファビューを取得する
     * @return D3D12_VERTEX_BUFFER_VIEW 描画に使用する頂点バッファビュー
     */
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return vertexBufferView_; }

    /**
     * @brief モデルが持つ総頂点数を取得する
     * @return size_t 頂点の総数
     */
    size_t GetVertexCount() const { return vertices_.size(); }

private:
    
    /**
     * @brief OBJファイルを解析し、頂点座標・UV・法線を読み込む内部関数
     * @param filePath 読み込むOBJファイルのパス
     */
    void LoadObjFile(const std::string& filePath);

    /** @brief 共通描画設定のポインタ（デバイスやコマンドリストの取得に使用） */
    ModelCommon* modelCommon_ = nullptr;

    /** @brief GPU側の頂点バッファリソース */
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    
    /** @brief 頂点バッファビュー（GPUに頂点データの位置とサイズを伝えるための構造体） */
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ {};

    /** @brief 使用するテクスチャのファイルパス */
    std::string textureFilePath_;

    /** @brief CPU側で保持するモデルの全頂点データの配列 */
    std::vector<VertexData> vertices_;
};