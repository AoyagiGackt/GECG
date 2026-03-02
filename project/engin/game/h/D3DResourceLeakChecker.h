/**
 * @file D3DResourceLeakChecker.h
 * @brief DirectX12のリソースリークを検出し、デバッグ出力に報告するクラスを定義するファイル
 */
#pragma once

/**
 * @brief DirectXリソースのリークチェックを行うクラス
 * @note このクラスのインスタンスを main 関数の最上位（最も外側のスコープ）で生成することで、
 * 全てのリソースが破棄された後の本当のアプリケーション終了時に、
 * 未解放のライブオブジェクト（リソース）をリストアップさせることができます
 */
class D3DResourceLeakChecker {
public:
    /**
     * @brief デストラクタ
     * @note インスタンスが破棄されるタイミング（通常は main 関数の終了時）で実行されます
     * DXGIのデバッグインターフェース（IDXGIDebug）を呼び出し、リーク情報を出力ウィンドウに報告します
     */
    ~D3DResourceLeakChecker();
};
