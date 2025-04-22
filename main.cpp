/*———————————–——————–——————–——————–——————–
*include
———————————–——————–——————–——————–——————–*/

#include <Windows.h>
#include <cstdint>
#include <format>
#include <string>
// 時間を扱うライブラリ
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>

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

HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

assert(SUCCEEDED(hr));

// 使用するアダプタ用の変数
IDXGIFactory7* useAdapter = nullptr;

// いい順にアダプタを頼む
for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DEGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; i++) {
    // アダプターの情報を取得する
    DXGI_ADAPTER_DESC1 adapterDesc;
    hr = useAdapter->GetDesc3(&adapterDesc);
    assert(SUCCEEDED(hr));
    // ソフトウェアアダプタでなければ採用!
    if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
        Log(std::format(L"Adapater:{}\n", adapterDesc.Description));
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
        Log(std::format("Feature Level: {}\n", featureLevelStrings[i]));
        break;
    }
}

// デバイスの生成に失敗したので起動できない
assert(device != nullptr);
Log("Complete create D3D12Device!!!\n"); // 初期化完了のログを出す

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

    // 出力ウィンドウへの文字入力
    OutputDebugStringA("Hello, DirectX!\n");

    Log(ConvertString(std::format(L"WSTRING{}\n", L"abc")));

    return 0;
}