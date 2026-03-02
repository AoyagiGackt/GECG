#pragma once

// すべてのゲーム内オブジェクトの共通の親クラス
class GameObject {
public:
    // 仮想デストラクタ
    virtual ~GameObject() = default;

    // 純粋仮想関数
    virtual void Update() = 0;
    virtual void Draw() = 0;
};