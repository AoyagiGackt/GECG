#include "Audio.h"
#include "Logger.h"
#include "StringUtlity.h"
#include <cassert>

using namespace Microsoft::WRL;

void Audio::Initialize()
{
    HRESULT hr;

    // MediaFoundationの初期化
    hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    assert(SUCCEEDED(hr));

    // XAudio2エンジンのインスタンス作成
    hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(hr));

    // マスターボイスの作成
    hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
    assert(SUCCEEDED(hr));

    Logger::Log("Audio System Initialized (Media Foundation & XAudio2).\n");
}

void Audio::Finalize()
{
    if (!xAudio2_) {
        return;
    }

    if (pSourceVoice_) {
        pSourceVoice_->DestroyVoice();
        pSourceVoice_ = nullptr;
    }

    // XAudio2の終了
    if (masteringVoice_) {
        masteringVoice_->DestroyVoice();
        masteringVoice_ = nullptr;
    }
    xAudio2_.Reset();

    // MediaFoundationの終了
    MFShutdown();
}

SoundData Audio::LoadAudio(const std::string& filename)
{
    HRESULT hr;
    SoundData soundData = {};

    // ソースリーダーの作成
    std::wstring wFilename = StringUtility::ConvertString(filename);
    ComPtr<IMFSourceReader> pSourceReader;
    hr = MFCreateSourceReaderFromURL(wFilename.c_str(), nullptr, &pSourceReader);
    if (FAILED(hr)) {
        Logger::Log("Error: Failed to open audio file: " + filename + "\n");
        assert(false);
        return soundData;
    }

    // メディアタイプの設定
    ComPtr<IMFMediaType> pMediaType;
    MFCreateMediaType(&pMediaType);
    pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    hr = pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMediaType.Get());
    if (FAILED(hr)) {
        Logger::Log("Error: Failed to set media type for: " + filename + "\n");
        assert(false);
        return soundData;
    }

    // 最終的なフォーマットを取得
    ComPtr<IMFMediaType> pOutputMediaType;
    hr = pSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutputMediaType);
    assert(SUCCEEDED(hr));

    WAVEFORMATEX* wfex = &soundData.wfex;
    wfex->wFormatTag = WAVE_FORMAT_PCM;

    UINT32 temp = 0;
    pOutputMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &temp);
    wfex->nChannels = (WORD)temp;

    pOutputMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &temp);
    wfex->nSamplesPerSec = temp;

    pOutputMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &temp);
    wfex->wBitsPerSample = (WORD)temp;

    pOutputMediaType->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &temp);
    wfex->nBlockAlign = (WORD)temp;

    pOutputMediaType->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &temp);
    wfex->nAvgBytesPerSec = temp;

    wfex->cbSize = 0;

    // データの読み込み
    std::vector<byte> rawData;
    while (true) {
        DWORD flags = 0;
        ComPtr<IMFSample> pSample;
        hr = pSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, &pSample);

        if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) {
            break;
        }

        if (!pSample) {
            continue;
        }

        ComPtr<IMFMediaBuffer> pBuffer;
        hr = pSample->ConvertToContiguousBuffer(&pBuffer);

        BYTE* pBufferPtr = nullptr;
        DWORD currentLength = 0;
        hr = pBuffer->Lock(&pBufferPtr, nullptr, &currentLength);

        size_t oldSize = rawData.size();
        rawData.resize(oldSize + currentLength);
        memcpy(rawData.data() + oldSize, pBufferPtr, currentLength);

        pBuffer->Unlock();
    }

    soundData.pBuffer = std::move(rawData);
    soundData.bufferSize = static_cast<unsigned int>(soundData.pBuffer.size());

    return soundData;
}

void Audio::PlayWave(const SoundData& soundData)
{
    HRESULT hr;

    // 前の音が鳴っていたら消す（BGM切り替え用）
    if (pSourceVoice_) {
        pSourceVoice_->Stop();
        pSourceVoice_->DestroyVoice();
        pSourceVoice_ = nullptr;
    }

    // 再生担当を作る
    hr = xAudio2_->CreateSourceVoice(&pSourceVoice_, &soundData.wfex);
    assert(SUCCEEDED(hr));

    // データをセット
    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = soundData.pBuffer.data();
    buffer.AudioBytes = soundData.bufferSize;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    hr = pSourceVoice_->SubmitSourceBuffer(&buffer);
    assert(SUCCEEDED(hr));

    // 再生開始
    hr = pSourceVoice_->Start(0);
}
