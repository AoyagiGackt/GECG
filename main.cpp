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

// DXGIファクトリーの生成
IDXGIFactory7* dxgiFactory = nullptr;

// 使用するアダプタ用の変数
IDXGIAdapter4* useAdapter = nullptr;

ID3D12Device* device = nullptr;

HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

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

// コマンドキューを生成する
ID3D12CommandQueue* commandQueue = nullptr;
D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};

// コマンドアロケータを生成する
ID3D12CommandAllocator* commandAllocator = nullptr;

// コマンドリストを生成する
ID3D12GraphicsCommandList* commandList = nullptr;

// スワップチェーンを生成する
IDXGISwapChain3* swapChain = nullptr;
DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

// ディスクリプタヒープの生成
ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};

// スワップチェーンからリソースを引っ張ってくる
ID3D12Resource* swapChainResoures[2] = { nullptr };

// RTVの設定
D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

// ディスクリプタの先頭を取得する
D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

// RTVを2つ分確保する
D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

typedef struct D3D12_CPU_DESCRIPTOR_HANDLE {
    UINT_PTR ptr;
} D3D12_CPU_DESCRIPTOR_HANDLE;

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

    // ウィンドウの×ボタンが押されるまでループ
    while (msg.message != WM_QUIT) {
        // windowsにメッセージが来てたら最優先で処理させる
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // ゲームの処理
        }
    }

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

    assert(SUCCEEDED(hr));

    // デバイスの生成に失敗したので起動できない
    assert(device != nullptr);

    // 適切なアダプタが見つからなかったので起動できない
    assert(useAdapter != nullptr);

    // コマンドキューの設定
    hr = device->CreateCommandQueue(
        &commandQueueDesc, // コマンドキューの設定
        IID_PPV_ARGS(&commandQueue) // コマンドキューのポインタ
    );

    // コマンドキューの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // コマンドアロケータの生成
    hr = device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストの種類
        IID_PPV_ARGS(&commandAllocator) // コマンドアロケータのポインタ
    );

    // コマンドアロケータの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

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
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用
    rtvDescriptorHeapDesc.NumDescriptors = 2; // ダブルバッファ用に2つ
    hr = device->CreateDescriptorHeap(
        &rtvDescriptorHeapDesc, // ディスクリプタヒープの設定
        IID_PPV_ARGS(&rtvDescriptorHeap) // ディスクリプタヒープのポインタ
    );

    // ディスクリプタヒープの生成に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // スワップチェーンのリソースを取得する
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResoures[0]));

    // スワップチェーンのリソースの取得に失敗したので起動できない
    assert(SUCCEEDED(hr));

    // スワップチェーンのリソースを取得する
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResoures[1]));

    // スワップチェーンのリソースの取得に失敗したので起動できない
    assert(SUCCEEDED(hr));

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 色の形式
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // テクスチャ2D

    // まず1つ目のスワップチェーンのリソースにRTVを設定する
    rtvHandles[0] = rtvStartHandle;
    device->CreateRenderTargetView(
        swapChainResoures[0], // スワップチェーンのリソース
        &rtvDesc, // RTVの設定
        rtvHandles[0] // RTVのハンドル
    );

    // 2つ目のスワップチェーンのリソースにRTVを設定する
    rtvHandles[1] = { rtvStartHandle.ptr * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) };

    // 2つ目を作る
    device->CreateRenderTargetView(
        swapChainResoures[1], // スワップチェーンのリソース
        &rtvDesc, // RTVの設定
        rtvHandles[1] // RTVのハンドル
    );

    rtvHandles[0] = rtvStartHandle;

    rtvHandles[1] = { rtvStartHandle.ptr * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) };

    // 出力ウィンドウへの文字入力
    OutputDebugStringA("Hello, DirectX!\n");

    Log(ConvertString(std::format(L"WSTRING{}\n", L"abc")));

    Log("Complete create D3D12Device!!!\n"); // 初期化完了のログを出す

    return 0;
}
