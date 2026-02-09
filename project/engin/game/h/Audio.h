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

struct SoundData {
    WAVEFORMATEX wfex; // 波形フォーマット
    std::vector<byte> pBuffer; // 音声バッファ
    unsigned int bufferSize; // バッファサイズ
};

class Audio {
public:
    // 初期化
    void Initialize();
    // 終了処理
    void Finalize();

    // 音系ファイルの読み込み
    SoundData LoadAudio(const std::string& filename);

    // 音声の再生
    void PlayWave(const SoundData& soundData);

private:
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    IXAudio2MasteringVoice* masteringVoice_ = nullptr;
    IXAudio2SourceVoice* pSourceVoice_ = nullptr;
};