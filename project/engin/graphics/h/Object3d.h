/**
 * @file Object3d.h
 * @brief 3D空間に配置される個々のオブジェクトを管理・描画するファイル
 */
#pragma once
#include "MakeAffine.h"
#include "Model.h"
#include <string>
#include <wrl/client.h>

class ModelCommon;
class Camera;

/**
 * @brief 3D空間に配置されるオブジェクトを表すクラス
 * @note 1つの Model（形状データ）を複数の Object3d で共有し、
 * それぞれ異なる座標（Transform）やマテリアルを持たせて描画することができます。
 */
class Object3d {
public:
    /**
     * @brief 全てのObject3dで共通して使用するデフォルトカメラを設定する
     * @param camera 共通カメラのポインタ
     * @note 個別のカメラがセットされていないオブジェクトは、このカメラを使用して描画されます。
     */
    static void SetCommonCamera(Camera* camera);

    /**
     * @brief オブジェクトの初期化。GPUに送るための定数バッファ（WVPやマテリアル用）を生成する
     * @param modelCommon 共通描画設定のポインタ
     */
    void Initialize(ModelCommon* modelCommon);

    /**
     * @brief 毎フレームの更新処理。トランスフォームからワールド行列とWVP行列を計算する
     */
    void Update();

    /**
     * @brief オブジェクトを描画するコマンドを積む
     * @note 事前に ModelCommon::CommonDrawSettings() が呼ばれている必要あり
     */
    void Draw();

    /**
     * @brief このオブジェクトに描画させるモデルの実体をセットする
     * @param model ModelManager等から取得したモデルのポインタ
     */
    void SetModel(Model* model) { model_ = model; }

    /**
     * @brief ファイルパスを指定して、ModelManagerからモデルを検索してセットする
     * @param filePath セットしたいモデルのファイルパス
     */
    void SetModel(const std::string& filePath);

    /**
     * @brief このオブジェクト専用のカメラをセットする（共通カメラを上書きする）
     * @param camera 個別に使用したいカメラのポインタ
     */
    void SetCamera(Camera* camera) { camera_ = camera; }

    /**
     * @brief オブジェクトの位置（座標）を設定する
     * @param position 新しいX, Y, Z座標
     */
    void SetPosition(const Vector3& position) { transform_.translate = position; }

    /**
     * @brief オブジェクトの回転角を設定する
     * @param rotation X, Y, Z軸の回転角度（ラジアン）
     */
    void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }

    /**
     * @brief オブジェクトのスケール（拡大縮小）を設定する
     * @param scale X, Y, Z軸のスケール倍率
     */
    void SetScale(const Vector3& scale) { transform_.scale = scale; }

    /**
     * @brief 現在のトランスフォーム（座標・回転・スケール）を取得する
     * @return const Transform& トランスフォーム構造体への参照
     */
    const Transform& GetTransform() const { return transform_; }

    /**
     * @brief トランスフォーム（座標・回転・スケール）への参照を取得する（書き込み用）
     * @return Transform& トランスフォームへの参照
     */
    Transform& GetTransform() { return transform_; }

private:
    /**
     * @brief GPUに送るための座標変換行列データ
     */
    struct TransformationMatrix {
        Matrix4x4 WVP; ///< ワールド・ビュー・プロジェクション行列
        Matrix4x4 World; ///< ワールド行列（法線やライティング計算用）
    };

    /**
     * @brief GPUに送るためのマテリアル（質感）データ
     */
    struct Material {
        Vector4 color; ///< 基本色（RGBA）
        int enableLighting; ///< ライティングを有効にするか（1:有効, 0:無効）
        int shadingType; ///< シェーディングの種類（ランバート、ハーフランバート等）
        float padding[2]; ///< HLSLの16バイトアライメント規則に合わせるためのパディング
        Matrix4x4 uvTransform; ///< UVアニメーション等に使うUV変換行列
    };

    /** @brief 全オブジェクトで共通して使うカメラのポインタ */
    static Camera* commonCamera_;

    /** @brief 共通描画設定のポインタ */
    ModelCommon* modelCommon_ = nullptr;

    /** @brief このオブジェクトが描画するモデルデータのポインタ */
    Model* model_ = nullptr;

    /** @brief このオブジェクト専用のカメラ（nullptrの場合は共通カメラを使う） */
    Camera* camera_ = nullptr;

    /** @brief オブジェクトのトランスフォーム（初期値はスケール1、原点） */
    Transform transform_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

    // --- GPUリソース関連 ---

    /** @brief 座標変換行列用のGPUリソース（定数バッファ） */
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;

    /** @brief CPU側で書き込むための座標変換行列データのポインタ（マップ済み） */
    TransformationMatrix* transformationMatrixData_ = nullptr;

    /** @brief マテリアル用のGPUリソース（定数バッファ） */
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

    /** @brief CPU側で書き込むためのマテリアルデータのポインタ（マップ済み） */
    Material* materialData_ = nullptr;
};