#include "Game.h"

// --------------------------------------------------
// メイン関数
// --------------------------------------------------

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // ゲームインスタンス生成
    Framework* game = new MyGame();

    // ゲーム実行
    game->Run();

    delete game;

    return 0;
}