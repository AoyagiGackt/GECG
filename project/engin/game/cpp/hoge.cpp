#include "hoge.h"

// コンストラクタ
Hoge::Hoge()
{
    // メンバ変数の初期化などが必要ならここに
}

// デストラクタ
Hoge::~Hoge()
{
    // 終了処理
    Finalize();
}

// 初期化処理
void Hoge::Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio)
{
    // 引数で受け取ったポインタをメンバ変数に保存する
    dxCommon_ = dxCommon;
    input_ = input;
    audio_ = audio;
}

// 更新処理
void Hoge::Update()
{

}

// 描画処理
void Hoge::Draw()
{

}

// 終了処理
void Hoge::Finalize()
{
    
}