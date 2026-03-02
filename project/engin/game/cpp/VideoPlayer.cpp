/**
 * @file VideoPlayer.cpp
 * @brief Media Foundationによる動画デコードとDX12テクスチャ転送の実装
 */
#include "VideoPlayer.h"
#include "SrvManager.h"
#include "StringUtlity.h"
#include <cassert>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

void VideoPlayer::Initialize(DirectXCommon* dxCommon)
{
    assert(dxCommon);
    dxCommon_ = dxCommon;

    // Media Foundationの開始（Audioですでに呼んでいる場合は重複しても問題なし）
    HRESULT hr = MFStartup(MF_VERSION);
    assert(SUCCEEDED(hr));
}

void VideoPlayer::Load(const std::string& filePath)
{
    HRESULT hr;
    std::wstring wpath = StringUtility::ConvertString(filePath);

    // ソースリーダーの作成
    hr = MFCreateSourceReaderFromURL(wpath.c_str(), nullptr, &sourceReader_);
    assert(SUCCEEDED(hr));

    // 出力形式を BGR32 (DX12の一般的なテクスチャ形式) に設定
    Microsoft::WRL::ComPtr<IMFMediaType> pType;
    hr = MFCreateMediaType(&pType);
    hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video); // 動画
    hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32); // RGB32形式
    hr = sourceReader_->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pType.Get());
    assert(SUCCEEDED(hr));

    // 動画の情報を取得（幅、高さ、フレームレート）
    Microsoft::WRL::ComPtr<IMFMediaType> pCurrentType;
    hr = sourceReader_->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pCurrentType);

    hr = MFGetAttributeSize(pCurrentType.Get(), MF_MT_FRAME_SIZE, &videoWidth_, &videoHeight_);

    UINT32 numerator, denominator;
    hr = MFGetAttributeRatio(pCurrentType.Get(), MF_MT_FRAME_RATE, &numerator, &denominator);
    if (numerator > 0) {
        frameDuration_ = 10000000LL * denominator / numerator; // 1秒(10^7)をFPSで割る
    }

    // 表示用のテクスチャを作成
    CreateVideoTexture(videoWidth_, videoHeight_);
}

void VideoPlayer::CreateVideoTexture(UINT width, UINT height)
{
    ID3D12Device* device = dxCommon_->GetDevice();

    // テクスチャの設定（書き換え頻度が高いため、通常はDefaultヒープに作成）
    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    resDesc.Width = width;
    resDesc.Height = height;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = { D3D12_HEAP_TYPE_DEFAULT };

    HRESULT hr = device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&videoTexture_));
    assert(SUCCEEDED(hr));

    // SRVの確保
    srvIndex_ = SrvManager::GetInstance()->Allocate();
    SrvManager::GetInstance()->CreateSRVforTexture2D(srvIndex_, videoTexture_.Get(), resDesc.Format, 1);
}

void VideoPlayer::Update()
{
    if (!isPlaying_ || !sourceReader_) {
        return;
    }

    HRESULT hr;
    DWORD streamIndex, flags;
    LONGLONG timestamp;
    Microsoft::WRL::ComPtr<IMFSample> pSample;

    // 1フレームデコード
    hr = sourceReader_->ReadSample(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0, &streamIndex, &flags, &timestamp, &pSample);

    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
        // ループ再生：最初に戻す
        PROPVARIANT var = {};
        var.vt = VT_I8;
        var.hVal.QuadPart = 0;
        sourceReader_->SetCurrentPosition(GUID_NULL, var);
        return;
    }

    if (pSample) {
        // GPUテクスチャへデータを転送
        Microsoft::WRL::ComPtr<IMFMediaBuffer> pBuffer;
        hr = pSample->ConvertToContiguousBuffer(&pBuffer);

        BYTE* pData = nullptr;
        DWORD cbLen = 0;
        hr = pBuffer->Lock(&pData, nullptr, &cbLen);

        // DX12の便利な機能を使ってピクセルデータをテクスチャへ流し込む
        // 1ピクセル4バイト (BGR32)
        dxCommon_->GetDevice();
        videoTexture_->WriteToSubresource(0, nullptr, pData, videoWidth_ * 4, cbLen);

        pBuffer->Unlock();
    }
}

void VideoPlayer::Finalize()
{
    sourceReader_.Reset();
    videoTexture_.Reset();
}