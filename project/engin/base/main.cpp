#include "D3DResourceLeakChecker.h"
#include "Game.h"

// --------------------------------------------------
// メイン関数
// --------------------------------------------------

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // リークチェックは冒頭に置いておく
    D3D12ResourceLeakChecker leakCheck;

    // ゲームインスタンス生成
    std::unique_ptr<Framework> game = std::make_unique<MyGame>();

    // ゲーム実行
    game->Run();

    return 0;
}