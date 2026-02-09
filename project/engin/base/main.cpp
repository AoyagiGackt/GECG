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
    std::unique_ptr<Game> game = std::make_unique<Game>();

    // 初期化
    game->Initialize();

    // メインループ
    while (true) {
        game->Update();

        // 終了リクエストがあれば抜ける
        if (game->IsEndRequest()) {
            break;
        }

        game->Draw();
    }

    // 終了処理
    game->Finalize();

    return 0;
}