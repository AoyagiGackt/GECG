/*———————————–——————–——————–——————–——————–
*include
———————————–——————–——————–——————–——————–*/

#include <Windows.h>
#include <cassert>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <format>
#include <string>

/*———————————–——————–——————–——————–——————–
*libのリンク
———————————–——————–——————–——————–——————–*/

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // メッセージに応じてゲーム固有の処理を行う
    switch (uMsg) {
        // ウィンドウが破棄された
    case WM_DESTROY:
        // OSに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;

    default:
        // 標準のメッセージ処理を行う
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

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

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

    WNDCLASS wc = {};

    // ウィンドクラスプロシージャ
    wc.lpfnWndProc = WindowProc;

    // ウィンドクラス名
    wc.lpszClassName = L"CG2WindowClass";

    // インスタンスハンドル
    wc.hInstance = GetModuleHandleW(nullptr);

    // カーソル
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    // ウィンドクラスの登録
    RegisterClassW(&wc);

    // クライアント領域のサイズ
    const int32_t kClientWidth = 1280;
    const int32_t kClientHeight = 720;

    // ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc = { 0, 0, kClientWidth, kClientHeight };

    // クライアント領域をもとに実際のサイズにwrcを変更
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    // ウィンドウの生成
    HWND hwnd = CreateWindow(
        wc.lpszClassName, // 利用するクラス名
        L"CG2", // タイトルバーの文字
        WS_OVERLAPPEDWINDOW, // よく見るウィンドウスタイル
        CW_USEDEFAULT, // 表示x座標(Windowsに任せる)
        CW_USEDEFAULT, // 表示y座標(WindowsOSに任せる)
        wrc.right - wrc.left, // ウィンドウ横幅
        wrc.bottom - wrc.top, // ウィンドウ縦幅
        nullptr, // 親ウィンドウハンドル
        nullptr, // メニューハンドル
        wc.hInstance, // インスタンスハンドル
        nullptr // オプション
    );

    // ウィンドウを表示する
    ShowWindow(hwnd, SW_SHOW);

    MSG msg {};

#ifdef _DEBUG
    ID3D12Debug1* debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        // デバッグレイヤーを有効にする
        debugController->EnableDebugLayer();
    } else {
        // さらにGPU側でもチェックを行うようにする
        debugController->SetEnableGPUBasedValidation(TRUE);
    }

#endif // DEBUG

    // DXGIファクトリーの生成
    IDXGIFactory7* dxgiFactory = nullptr;

    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

    assert(SUCCEEDED(hr));

    // 使用するアダプタ用の変数
    IDXGIAdapter4* useAdapter = nullptr;

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

    ID3D12Device* device = nullptr;

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
            useAdapter, // アダプタ
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
    ID3D12CommandQueue* commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};

    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

    // コマンドキューの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // コマンドアロケータを生成する
    ID3D12CommandAllocator* commandAllocator = nullptr;

    // コマンドアロケータの生成
    hr = device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストの種類
        IID_PPV_ARGS(&commandAllocator) // コマンドアロケータのポインタ
    );

    // コマンドアロケータの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // コマンドリストを生成する
    ID3D12GraphicsCommandList* commandList = nullptr;

    // コマンドリストの生成
    hr = device->CreateCommandList(
        0, // コマンドリストのフラグ
        D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストの種類
        commandAllocator, // コマンドアロケータ
        nullptr, // パイプラインステートオブジェクト
        IID_PPV_ARGS(&commandList) // コマンドリストのポインタ
    );

    // コマンドリストの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // スワップチェーンを生成する
    IDXGISwapChain4* swapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

    // スワップチェーンの設定
    swapChainDesc.Width = kClientWidth; // 画面の幅
    swapChainDesc.Height = kClientHeight; // 画面の高さ
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
    swapChainDesc.SampleDesc.Count = 1; // マルチサンプリングしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2; // ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // スワップ効果
    // コマンドキュー、ウィンドウハンドル、スワップチェーンの設定
    hr = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue, // コマンドキュー
        hwnd, // ウィンドウハンドル
        &swapChainDesc, // スワップチェーンの設定
        nullptr, // モニターのハンドル
        nullptr, // スワップチェーンのフラグ
        reinterpret_cast<IDXGISwapChain1**>(&swapChain) // スワップチェーンのポインタ
    );

    assert(SUCCEEDED(hr));

    // ディスクリプタヒープの生成
    ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};

    // ディスクリプタヒープの生成
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用
    rtvDescriptorHeapDesc.NumDescriptors = 2; // ダブルバッファ用に2つ
    hr = device->CreateDescriptorHeap(
        &rtvDescriptorHeapDesc, // ディスクリプタヒープの設定
        IID_PPV_ARGS(&rtvDescriptorHeap) // ディスクリプタヒープのポインタ
    );

    // ディスクリプタヒープの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // スワップチェーンからリソースを引っ張ってくる
    ID3D12Resource* swapChainResoures[2] = { nullptr };

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
    device->CreateRenderTargetView(swapChainResoures[0], &rtvDesc, rtvHandles[0]);

    // 2つ目のスワップチェーンのリソースにRTVを設定する
    rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 2つ目を作る
    device->CreateRenderTargetView(swapChainResoures[1], &rtvDesc, rtvHandles[1]);

    // ウィンドウの×ボタンが押されるまでループ
    while (msg.message != WM_QUIT) {
        // windowsにメッセージが来てたら最優先で処理させる
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {

            // ゲームの処理
            UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex(); // バックバッファのインデックス

            // 描画先のRTVを取得する
            commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);

            // TransitionBarrierの設定
            D3D12_RESOURCE_BARRIER barrier {};

            // 今回のバリアはTransition
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

            // Noneにする
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

            // バリアを張る対象のリソース。現在のバックバッファに対して行う
            barrier.Transition.pResource = swapChainResoures[backBufferIndex];

            // 漂移前のResourceState
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

            // 漂移後のResourceState
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

            // TransitionBarrierを張る
            commandList->ResourceBarrier(1, &barrier);

            // 指定した色で画面全体をクリアする
            float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f }; // 青っぽい色、RGBAの順

            commandList->ClearRenderTargetView(
                rtvHandles[backBufferIndex], // 描画先のRTV
                clearColor, // クリアする色
                0, // フラグ
                nullptr // 深度ステンシルビューのハンドル
            );

            // 今回はRenderTargetからPresentにする
            barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

            // TransitionBarrierを張る
            commandList->ResourceBarrier(1, &barrier);

            hr = commandList->Close();

            // コマンドリストの生成に失敗したので起動できない
            assert(SUCCEEDED(hr));

            // GPUのコマンドリスト実行を行わせる
            ID3D12CommandList* commandLists[] = { commandList };

            commandQueue->ExecuteCommandLists(1, commandLists);

            // GPUとOSに画面の交換を行うように通知する
            swapChain->Present(1, 0);

            // 次のフレーム用のコマンドリストを準備
            hr = commandAllocator->Reset();
            assert(SUCCEEDED(hr));
            hr = commandList->Reset(commandAllocator, nullptr);
            assert(SUCCEEDED(hr));
        }
    }

    // 出力ウィンドウへの文字入力
    OutputDebugStringA("Hello, DirectX!\n");

    Log(ConvertString(std::format(L"WSTRING{}\n", L"abc")));

    Log("Complete create D3D12Device!!!\n"); // 初期化完了のログを出す

#ifdef _DEBUG
    ID3D12InfoQueue* infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        // デバッグレイヤーのメッセージを全て出力する
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        // エラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        // 警告時に泊まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        // 抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            // Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };

        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

        D3D12_INFO_QUEUE_FILTER filter {};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;

        // 指定したメッセージの表示を無力化する
        infoQueue->PushStorageFilter(&filter);

        // 解放
        infoQueue->Release();
    }
#endif

    return 0;
}
