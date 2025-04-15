/*———————————–———————————–—————————–——————
*  include
———————————–———————————–—————————–————————*/
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

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // 出力ウィンドウへの文字入力
    OutputDebugStringA("Hello, DirectX!\n");
    return 0;
}