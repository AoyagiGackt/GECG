#pragma once
#include "Camera.h"
#include "DirectXCommon.h"
#include "MakeAffine.h"
#include "Model.h"
#include "SrvManager.h"
#include <d3d12.h>
#include <list>
#include <string>
#include <wrl/client.h>

// パーティクルのデータ
struct Particle {
    Transform transform;
    Vector3 velocity;
    Vector4 color;
    float lifeTime;
    float currentTime;
};

// GPUに送るデータ (StructuredBuffer用)
struct ParticleForGPU {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
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
    // インスタンシング用バッファの作成
    void CreateInstancingResource();

private:
    DirectXCommon* dxCommon_ = nullptr;

    // パイプライン関連
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

    // インスタンシング用リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
    ParticleForGPU* instancingData_ = nullptr;
    uint32_t srvIndex_ = 0; // SRVのインデックス
    const uint32_t kNumMaxInstance = 1024; // 最大描画数

    // パーティクルリスト
    std::list<Particle> particles_;

    // 描画するモデル
    Model* model_ = nullptr;
};