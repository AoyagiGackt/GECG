/**
 * @file Camera.h
 * @brief カメラの座標やビュー・プロジェクション行列を管理するファイル
 */
#pragma once
#include "MakeAffine.h"

/**
 * @brief 3D空間のカメラを表現するクラス
 */
class Camera {
public:
    /**
     * @brief コンストラクタ。初期パラメータ（座標や視野角など）の設定を行う。
     */
    Camera();

    /**
     * @brief カメラの各種行列を更新する
     * @note 毎フレーム、トランスフォームの変更後に必ず呼び出さないといけない
     */
    void Update();

    /**
     * @brief カメラの回転角を設定する
     * @param rotate X, Y, Z軸の回転角度（ラジアン）
     */
    void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
    
    /**
     * @brief カメラの位置を設定する
     * @param translate 新しいX, Y, Z座標
     */
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
    
    /**
     * @brief 垂直画角（Field of View Y）を設定する
     * @param fovY 視野角（ラジアン）
     */
    void SetFovY(float fovY) { fovY_ = fovY; }
    
    /**
     * @brief アスペクト比（画面の縦横比）を設定する
     * @param aspectRatio アスペクト比（横幅 / 縦幅）
     */
    void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
    
    /**
     * @brief ニアクリップ面（これより近いものは描画されない）を設定する
     * @param nearClip ニアクリップ距離
     */
    void SetNearClip(float nearClip) { nearClip_ = nearClip; }
    
    /**
     * @brief ファークリップ面（これより遠いものは描画されない）を設定する
     * @param farClip ファークリップ距離
     */
    void SetFarClip(float farClip) { farClip_ = farClip; }

    /**
     * @brief カメラの回転角を取得する（読み取り専用）
     * @return const Vector3& 現在の回転角（ラジアン）
     */
    const Vector3& GetRotate() const { return transform_.rotate; }
    
    /**
     * @brief カメラの位置を取得する（読み取り専用）
     * @return const Vector3& 現在の位置座標
     */
    const Vector3& GetTranslate() const { return transform_.translate; }
    
    /**
     * @brief 計算済みのビュー行列を取得する
     * @return const Matrix4x4& ビュー行列
     */
    const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
    
    /**
     * @brief 計算済みのプロジェクション行列を取得する
     * @return const Matrix4x4& プロジェクション行列
     */
    const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }

    /**
     * @brief カメラの回転角を参照取得する（直接変更可能）
     * @return Vector3& 回転角への参照
     */
    Vector3& GetRotate() { return transform_.rotate; }
    
    /**
     * @brief カメラの位置を参照取得する（直接変更可能）
     * @return Vector3& 位置座標への参照
     */
    Vector3& GetTranslate() { return transform_.translate; }

private:
    /** @brief カメラのトランスフォーム（座標・回転・スケール） */
    Transform transform_;

    /** @brief ビュー行列（カメラのワールド行列の逆行列） */
    Matrix4x4 viewMatrix_;

    /** @brief プロジェクション行列（透視投影行列） */
    Matrix4x4 projectionMatrix_;

    // プロジェクション行列用パラメータ
    /** @brief 垂直画角（ラジアン） */
    float fovY_;

    /** @brief アスペクト比（画面の横幅 / 縦幅） */
    float aspectRatio_;
    
    /** @brief ニアクリップ距離 */
    float nearClip_;

    /** @brief ファークリップ距離 */
    float farClip_;
};