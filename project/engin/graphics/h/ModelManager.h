#pragma once
#include "Model.h"
#include "ModelCommon.h"
#include <map>
#include <memory>
#include <string>

class ModelManager {
public:
    // インスタンス取得
    static ModelManager* GetInstance();

    // 初期化
    void Initialize(ModelCommon* modelCommon);

    // モデルの読み込み
    void LoadModel(const std::string& filePath, const std::string& textureFilePath);

    // モデルの検索取得
    Model* FindModel(const std::string& filePath);

    // 終了処理
    void Finalize();

private:
    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;
    ModelCommon* modelCommon_ = nullptr;

    std::map<std::string, std::unique_ptr<Model>> models_;
};