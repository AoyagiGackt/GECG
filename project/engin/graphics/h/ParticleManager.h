#pragma once
#include "Camera.h"
#include "DirectXCommon.h"
#include "MakeAffine.h"
#include "Model.h"
#include "SrvManager.h"
#include <d3d12.h>
#include <list>
#include <string>
#include <unordered_map>
#include <wrl/client.h>

// パーティクル1粒のデータ
struct Particle {
    Transform transform;
    Vector3 velocity;
    Vector4 color;
    float lifeTime;
    float currentTime;
};

// GPUに送るデータ
struct ParticleForGPU {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
};

// パーティクルグループ
struct ParticleGroup {
    // マテリアルデータ
    std::string textureFilePath;

    // パーティクルリスト
    std::list<Particle> particles;

    // インスタンシングデータ用SRVのインデックス
    uint32_t srvIndex = 0;

    // インスタンシング用リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;

    // インスタンス数
    const uint32_t kNumMaxInstance = 1024;

    // インスタンシングデータを書き込むためのポインタ
    ParticleForGPU* instancingData = nullptr;
};

class ParticleManager {
public:
    static ParticleManager* GetInstance();

    // 初期化
    void Initialize(DirectXCommon* dxCommon);

    // 更新
    void Update(Camera* camera);

    // 描画
    void Draw(Camera* camera);

    // パーティクルの発生
    void Emit(const std::string& name, const Vector3& position, const Vector3& velocity);

    // パーティクルグループの生成
    void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);

    // モデルのセット (パーティクルの形状)
    void SetModel(Model* model) { model_ = model; }

private:
    ParticleManager() = default;
    ~ParticleManager() = default;
    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;

    // ルートシグネチャの作成
    void CreateRootSignature();
    // グラフィックスパイプラインの作成
    void CreatePipelineState();

private:
    DirectXCommon* dxCommon_ = nullptr;

    // パイプライン関連
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

    std::unordered_map<std::string, ParticleGroup> particleGroups_;

    Model* model_ = nullptr;
};