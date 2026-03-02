/**
 * @file VideoPlayer.h
 * @brief Media Foundationを使用して動画をデコードし、DX12テクスチャとして提供するクラス
 */
#pragma once
#include "DirectXCommon.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <string>
#include <vector>
#include <wrl/client.h>

/**
 * @brief 動画再生管理クラス
 */
class VideoPlayer {
public:
    /**
     * @brief 初期化
     * @param dxCommon DirectX基盤のポインタ
     */
    void Initialize(DirectXCommon* dxCommon);

    /**
     * @brief 動画ファイルのオープンとテクスチャの準備
     * @param filePath 動画ファイルのパス
     */
    void Load(const std::string& filePath);

    /**
     * @brief 毎フレームの更新処理
     * @note 内部で動画のフレームレートに合わせてデコードとテクスチャ更新を行います
     */
    void Update();

    /** @brief 再生開始 */
    void Play() { isPlaying_ = true; }

    /** @brief 一時停止 */
    void Pause() { isPlaying_ = false; }

    /** @brief 終了処理 */
    void Finalize();

    /** @brief 動画テクスチャのSRVインデックスを取得（SrvManagerで使用） */
    uint32_t GetSrvIndex() const { return srvIndex_; }

    /** @brief 現在のフレームのリソースを取得 */
    ID3D12Resource* GetResource() const { return videoTexture_.Get(); }

private:
    /** @brief 動画フレームを保持するテクスチャリソースの作成 */
    void CreateVideoTexture(UINT width, UINT height);

private:
    DirectXCommon* dxCommon_ = nullptr;

    // Media Foundation 関連
    Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader_;

    // DirectX12 関連
    Microsoft::WRL::ComPtr<ID3D12Resource> videoTexture_;
    uint32_t srvIndex_ = 0;

    // 再生制御
    bool isPlaying_ = false;
    UINT videoWidth_ = 0;
    UINT videoHeight_ = 0;
    LONGLONG frameDuration_ = 0; ///< 1フレームの時間
    LONGLONG nextFrameTime_ = 0; ///< 次のフレームを表示すべき時刻
};