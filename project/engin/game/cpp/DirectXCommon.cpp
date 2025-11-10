#include "DirectXCommon.h"
#include "WinApp.h"
#include <cassert>
#include <format>
#include <string>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// 文字列を出す
void Log(const std::string& message)
{
    OutputDebugStringA(message.c_str());
}

std::wstring ConvertString(const std::string& str)
{
    if (str.empty()) {
        return std::wstring();
    }
    auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
    if (sizeNeeded == 0) {
        return std::wstring();
    }
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
    return result;
}

std::string ConvertString(const std::wstring& str)
{
    if (str.empty()) {
        return std::string();
    }
    auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0) {
        return std::string();
    }
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
    return result;
}

ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
    ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
    // ディスクリプタヒープの生成
    ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    // ディスクリプタヒープの生成
    descriptorHeapDesc.Type = heapType; // レンダーターゲットビュー用
    descriptorHeapDesc.NumDescriptors = numDescriptors; // ダブルバッファ用に2つ
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = device->CreateDescriptorHeap(
        &descriptorHeapDesc, // ディスクリプタヒープの設定
        IID_PPV_ARGS(&descriptorHeap) // ディスクリプタヒープのポインタ
    );

    // ディスクリプタヒープの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}

void DirectXCommon::Initialize()
{

#ifdef _DEBUG
    ComPtr<ID3D12Debug1> debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        // デバッグレイヤーを有効にする
        debugController->EnableDebugLayer();
    } else {
        // さらにGPU側でもチェックを行うようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }

#endif // DEBUG

    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

    assert(SUCCEEDED(hr));

    // 使用するアダプタ用の変数
    ComPtr<IDXGIAdapter4> useAdapter = nullptr;

    // いい順にアダプタを頼む
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
        // アダプターの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc {};
        hr = useAdapter->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr));

        // ソフトウェアアダプタでなければ採用!
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
            Log(ConvertString(std::format(L"USE Adapter:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
    }

    // 適切なアダプタが見つからなかったので起動できない
    assert(useAdapter != nullptr);

    // 機能レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
    };

    const char* featureLevelStrings[] = {
        "12.2",
        "12.1",
        "12.0",
    };

    // 高い順に生成できるか試していく
    for (size_t i = 0; i < _countof(featureLevels); i++) {
        // 採用したアダプターでデバイスの生成
        hr = D3D12CreateDevice(
            useAdapter.Get(), // アダプタ
            featureLevels[i], // 機能レベル
            IID_PPV_ARGS(&device) // デバイス
        );

        // 指定した機能レベルでログ出力を行ってループを抜ける
        if (SUCCEEDED(hr)) {
            Log((std::format("Feature Level: {}\n", featureLevelStrings[i])));
            break;
        }
    }

    // デバイスの生成に失敗したので起動できない
    assert(device != nullptr);

    // コマンドキューを生成する
    ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};

    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

    // コマンドキューの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // コマンドアロケータを生成する
    ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;

    // コマンドアロケータの生成
    hr = device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストの種類
        IID_PPV_ARGS(&commandAllocator) // コマンドアロケータのポインタ
    );

    // コマンドアロケータの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // コマンドリストを生成する
    ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

    // コマンドリストの生成
    hr = device->CreateCommandList(
        0, // コマンドリストのフラグ
        D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストの種類
        commandAllocator.Get(), // コマンドアロケータ
        nullptr, // パイプラインステートオブジェクト
        IID_PPV_ARGS(&commandList) // コマンドリストのポインタ
    );

    // コマンドリストの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // スワップチェーンを生成する
    ComPtr<IDXGISwapChain4> swapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

    // スワップチェーンの設定
    swapChainDesc.Width = WinApp::kClientWidth; // 画面の幅
    swapChainDesc.Height = WinApp::kClientHeight; // 画面の高さ
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
    swapChainDesc.SampleDesc.Count = 1; // マルチサンプリングしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2; // ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // スワップ効果
    // コマンドキュー、ウィンドウハンドル、スワップチェーンの設定
    hr = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr,
        reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));

    assert(SUCCEEDED(hr));

    ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(
        device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

    ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(
        device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

    // スワップチェーンからリソースを引っ張ってくる
    ComPtr<ID3D12Resource> swapChainResoures[2] = { nullptr };

    // スワップチェーンのリソースを取得する
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResoures[0]));

    // スワップチェーンのリソースの取得に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // スワップチェーンのリソースを取得する
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResoures[1]));

    // スワップチェーンのリソースの取得に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // RTVの設定
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 色の形式
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // テクスチャ2D

    // ディスクリプタの先頭を取得する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    // RTVを2つ分確保する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

    // まず1つ目のスワップチェーンのリソースにRTVを設定する
    rtvHandles[0] = rtvStartHandle;
    device->CreateRenderTargetView(swapChainResoures[0].Get(), &rtvDesc, rtvHandles[0]);

    // 2つ目のスワップチェーンのリソースにRTVを設定する
    rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 2つ目を作る
    device->CreateRenderTargetView(swapChainResoures[1].Get(), &rtvDesc, rtvHandles[1]);

    // 初期値0でFrenceを作る
    ComPtr<ID3D12Fence> fence = nullptr;
    uint64_t fenceValue = 0;
    hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));

    // FenceのSignalを持つためのイベントを作成する
    HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);
}