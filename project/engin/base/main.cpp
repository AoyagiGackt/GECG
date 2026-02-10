#include "Game.h"
#include <memory>

// --------------------------------------------------
// メイン関数
// --------------------------------------------------

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // ゲームインスタンス生成
    std::unique_ptr<Framework> game = std::make_unique<MyGame>();

    // ゲーム実行
    game->Run();

    return 0;
}