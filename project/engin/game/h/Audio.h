/**
 * @file Audio.h
 * @brief XAudio2を使用した音声ファイルの読み込み・再生を管理するファイル
 */
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <xaudio2.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfuuid.lib")

/**
 * @brief 再生する音声データを保持する構造体
 */
struct SoundData {
    WAVEFORMATEX wfex; ///< 波形フォーマット（サンプリングレートやチャンネル数など）
    std::vector<byte> pBuffer; ///< 音声データのバッファ本体（std::vectorによる自動メモリ管理）
    unsigned int bufferSize; ///< バッファの総サイズ（バイト単位）
};

/**
 * @brief オーディオ再生を管理するクラス
 * @note XAudio2 による音声出力と、Media Foundation による柔軟なファイルロードにする
 */
class Audio {
public:
    
    /**
     * @brief オーディオエンジンの初期化
     * @note XAudio2 デバイスの作成、および Media Foundation の初期化（MFStartup）を行います
     */
    void Initialize();
    
    /**
     * @brief オーディオエンジンの終了処理
     * @note 作成した音声の破棄、XAudio2 の解放、および Media Foundation の終了処理（MFShutdown）を行います
     */
    void Finalize();

    /**
     * @brief 音声ファイルを読み込む
     * @param filename 読み込む音声ファイルのパス（例: "Resources/bgm.mp3" や "Resources/se.wav"）
     * @return SoundData 読み込んだ音声データ
     * @note Media Foundation を使用しているため、WAV 以外の形式（MP3/WMA等）もロード可能
     */
    SoundData LoadAudio(const std::string& filename);

    /**
     * @brief 音声を再生する
     * @param soundData 再生したい音声データ
     * @note 内部でソースボイスを生成し、バッファを送信して再生を開始します
     */
    void PlayWave(const SoundData& soundData);

private:

    /** @brief XAudio2 エンジンの本体ポインタ */
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

    /** @brief 最終的な音の出口となるもの */
    IXAudio2MasteringVoice* masteringVoice_ = nullptr;

    /** @brief 音声データの供給源となるもの */
    IXAudio2SourceVoice* pSourceVoice_ = nullptr;
};