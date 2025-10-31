#include "WinApp.h"
#include "imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ウィンドウプロシージャ
LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
        return true;
    }

    // メッセージに応じてゲーム固有の処理を行う
    switch (uMsg) {
        // ウィンドウが破棄された
    case WM_DESTROY:
        // OSに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;
    }

    // 標準のメッセージ処理を行う
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void WinApp::Initialize()
{
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

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

    // ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc = { 0, 0, kClientWidth, kClientHeight };

    // クライアント領域をもとに実際のサイズにwrcを変更
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    hwnd = CreateWindow(
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
}

void WinApp::Update()
{
   
}

void WinApp::Finalize()
{
    CloseWindow(hwnd);
    CoUninitialize();
}

bool WinApp::ProcessMessage()
{
    MSG msg {};

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (msg.message == WM_QUIT) {
        return true;
    }

    return false;
}
