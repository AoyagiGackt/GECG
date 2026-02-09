#include "Audio.h"
#include "Logger.h"
#include <cassert>
#include <fstream>

#pragma comment(lib, "xaudio2.lib")

struct ChunkHeader {
    char id[4];
    int32_t size;
};

struct RiffHeader {
    ChunkHeader chunk;
    char type[4];
};

void Audio::Initialize()
{
    // XAudio2エンジンのインスタンス作成
    HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(hr));

    // マスターボイスの作成
    hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
    assert(SUCCEEDED(hr));

    Logger::Log("Audio System Initialized.\n");
}

void Audio::Finalize()
{
    if (masteringVoice_) {
        masteringVoice_->DestroyVoice();
        masteringVoice_ = nullptr;
    }
    xAudio2_.Reset();
}

SoundData Audio::LoadWave(const std::string& filename)
{
    std::ifstream file;
    file.open(filename, std::ios_base::binary);
    assert(file.is_open());

    // RIFFチャンクの読み込み
    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    assert(strncmp(riff.chunk.id, "RIFF", 4) == 0);
    assert(strncmp(riff.type, "WAVE", 4) == 0);

    SoundData soundData = {};

    // チャンクを順番に読み込む
    while (true) {
        ChunkHeader chunk;
        file.read((char*)&chunk, sizeof(chunk));
        if (file.eof()) {
            break;
        }

        if (strncmp(chunk.id, "fmt ", 4) == 0) {
            // fmtチャンクの読み込み
            file.read((char*)&soundData.wfex, chunk.size);
        } else if (strncmp(chunk.id, "data", 4) == 0) {
            // dataチャンクの読み込み
            soundData.bufferSize = chunk.size;
            soundData.pBuffer.resize(chunk.size);
            file.read((char*)soundData.pBuffer.data(), chunk.size);
        } else {
            // 不要なチャンクを飛ばす
            file.seekg(chunk.size, std::ios_base::cur);
        }
    }
    file.close();
    return soundData;
}

void Audio::PlayWave(const SoundData& soundData)
{
    HRESULT hr;
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    hr = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
    assert(SUCCEEDED(hr));

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = soundData.pBuffer.data();
    buffer.AudioBytes = soundData.bufferSize;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    hr = pSourceVoice->SubmitSourceBuffer(&buffer);
    assert(SUCCEEDED(hr));

    hr = pSourceVoice->Start(0);
}