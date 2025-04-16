/*———————————–——————–——————–——————–——————–
*include
———————————–——————–——————–——————–——————–*/

#include <Windows.h>
#include <cstdint>
#include <format>
#include <string>
// ファイルやディレクトリに関する操作を行うライブラリ
#include <filesystem>
// ファイルに書いたり読んだりするライブラリ
#include <fstream>
// 時間を扱うライブラリ
#include <chrono>
#include<d3d12.h>
#include <dxgi1_4.h>
#include <cassert>

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

void Log(std::ostream& os, const std::string& message)
{
    // ログファイルに書き込む
    os << message << std::endl;
    // 出力ウィンドウに書き込む
    OutputDebugStringA(message.c_str());
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

    // string->wstring
    std::wstring ConvertString(const std::string& str);

    // wstring->string
    std::string ConvertString(const std::wstring& str);

    // ログのディレクトリを用意
    std::filesystem::create_directory("logs");

    // 現在時間を取得
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    // ログファイルの名前にコンマ何秒入らないので削って秒にする
    std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
        now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

    // 日本時間(PCの時間)に変換
    std::chrono::zoned_time localTime(std::chrono::current_zone(), now_seconds);

    // formztを使って年月日_時分秒の文字列に変換
    std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

    // 時刻を使ってファイル名を決定
    std::string logFilePath = std::string("logs/") + dateString + ".log";

    // ファイルを作って書き込み準備
    std::ofstream logFile(logFilePath);

    // 出力ウィンドウへの文字入力
    OutputDebugStringA("Hello, DirectX!\n");

    return 0;
}