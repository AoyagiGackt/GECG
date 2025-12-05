#include "ParticleManager.h"
#include "TextureManager.h"
#include <cassert>
#include <d3dx12.h>

using namespace Microsoft::WRL;

ParticleManager* ParticleManager::GetInstance()
{
    static ParticleManager instance;
    return &instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon)
{
    assert(dxCommon);
    dxCommon_ = dxCommon;

    CreateRootSignature();
    CreatePipelineState();
    CreateInstancingResource();
}

void ParticleManager::Update(Camera* camera)
{
    Matrix4x4 viewProj = Multiply(camera->GetViewMatrix(), camera->GetProjectionMatrix());
    // ビルボード用の行列（カメラの回転だけを適用した行列の逆行列）
    Matrix4x4 billboardMatrix = MakeIdentity4x4();
    /* 本来はカメラの回転行列を取得して計算するが、
       簡易的にカメラのView行列の回転成分の逆行列を使用する実装例
    */
    Matrix4x4 cameraView = camera->GetViewMatrix();
    billboardMatrix.m[0][0] = cameraView.m[0][0];
    billboardMatrix.m[0][1] = cameraView.m[1][0];
    billboardMatrix.m[0][2] = cameraView.m[2][0];
    billboardMatrix.m[1][0] = cameraView.m[0][1];
    billboardMatrix.m[1][1] = cameraView.m[1][1];
    billboardMatrix.m[1][2] = cameraView.m[2][1];
    billboardMatrix.m[2][0] = cameraView.m[0][2];
    billboardMatrix.m[2][1] = cameraView.m[1][2];
    billboardMatrix.m[2][2] = cameraView.m[2][2];

    uint32_t instanceCount = 0;

    for (auto it = particles_.begin(); it != particles_.end();) {
        // 寿命チェック
        if (it->currentTime >= it->lifeTime) {
            it = particles_.erase(it);
            continue;
        }

        // 移動処理
        it->transform.translate.x += it->velocity.x;
        it->transform.translate.y += it->velocity.y;
        it->transform.translate.z += it->velocity.z;

        // 経過時間加算
        it->currentTime += 1.0f / 60.0f;

        // GPU送信用データの作成
        if (instanceCount < kNumMaxInstance) {
            // スケール行列
            Matrix4x4 scaleMatrix = MakeScaleMatrix(it->transform.scale);
            // 平行移動行列
            Matrix4x4 translateMatrix = MakeTranslateMatrix(it->transform.translate);

            // ワールド行列 (ビルボード回転 * スケール * 平行移動)
            // 回転はビルボード行列を使用する
            Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(billboardMatrix, translateMatrix));

            instancingData_[instanceCount].WVP = Multiply(worldMatrix, viewProj);
            instancingData_[instanceCount].World = worldMatrix;
            instancingData_[instanceCount].color = it->color;

            // アルファ値を寿命に合わせてフェードアウトさせる例
            float alpha = 1.0f - (it->currentTime / it->lifeTime);
            instancingData_[instanceCount].color.w = alpha;

            instanceCount++;
        }
        ++it;
    }
}

void ParticleManager::Draw(Camera* camera)
{
    if (!model_)
        return;

    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

    // パイプライン設定
    commandList->SetGraphicsRootSignature(rootSignature_.Get());
    commandList->SetPipelineState(graphicsPipelineState_.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // モデルの頂点バッファをセット (Modelクラスの実装に依存するが、VBV取得が必要)
    // Modelクラスに GetVertexBufferView() がある前提、もしくは Draw 内で設定してもらう
    // 今回は Model::Draw を使わず、ここでコマンドを組むため、ModelからVBVなどを取得する必要があります。
    // ※Modelクラスの改修を避けるため、簡易的にModel::Drawの一部を模倣します。
    // 実際には Model クラスに Getter を追加してください。

    // --- StructuredBuffer (SRV) をセット (RootParameter[0]) ---
    SrvManager::GetInstance()->PreDraw();
    commandList->SetGraphicsRootDescriptorTable(0, SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex_));

    // --- テクスチャ (SRV) をセット (RootParameter[1]) ---
    // モデルが持っているテクスチャパスを使用する想定
    // Model::Draw は通常 DrawInstanced(vertices, 1, ...) を呼ぶため、ここでは使えない。
    // そのため、自前でDrawInstancedを呼ぶ。

    // (注: Modelクラスに getter が必要です。以下は仮想コードです)
    // commandList->IASetVertexBuffers(0, 1, &model_->GetVertexBufferView());
    // D3D12_GPU_DESCRIPTOR_HANDLE textureH = TextureManager::GetInstance()->GetSrvHandleGPU(model_->GetTextureFilePath());
    // commandList->SetGraphicsRootDescriptorTable(1, textureH);

    // 現在のパーティクル数分だけインスタンシング描画
    // 頂点数は四角形ポリゴン(6頂点)と仮定
    commandList->DrawInstanced(6, (UINT)particles_.size(), 0, 0);
}

void ParticleManager::Emit(const std::string& name, const Vector3& position, const Vector3& velocity)
{
    // 新しいパーティクルを生成
    Particle newParticle;
    newParticle.transform.scale = { 1.0f, 1.0f, 1.0f };
    newParticle.transform.rotate = { 0.0f, 0.0f, 0.0f };
    newParticle.transform.translate = position;
    newParticle.velocity = velocity;
    newParticle.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    newParticle.lifeTime = 1.0f; // 1秒生存
    newParticle.currentTime = 0.0f;

    particles_.push_back(newParticle);
}

void ParticleManager::CreateRootSignature()
{
    ID3D12Device* device = dxCommon_->GetDevice();

    D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
    // StructuredBuffer (t0)
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // Texture (t1)
    descriptorRange[1].BaseShaderRegister = 1;
    descriptorRange[1].NumDescriptors = 1;
    descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[2] = {};
    // Parameter 0: StructuredBuffer
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // VSでもPSでも使う
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // Parameter 1: Texture
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // サンプラー設定
    D3D12_STATIC_SAMPLER_DESC staticSampler = {};
    staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.ShaderRegister = 0;
    staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature {};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);
    descriptionRootSignature.pStaticSamplers = &staticSampler;
    descriptionRootSignature.NumStaticSamplers = 1;

    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        assert(false);
    }
    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void ParticleManager::CreatePipelineState()
{
    ID3D12Device* device = dxCommon_->GetDevice();

    // シェーダーコンパイル
    // ファイル名は適宜プロジェクトに合わせてください
    IDxcBlob* vsBlob = dxCommon_->CompileShader(L"Resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    IDxcBlob* psBlob = dxCommon_->CompileShader(L"Resources/shaders/Particle.PS.hlsl", L"ps_6_0");

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc {};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

    // ブレンドステート (加算合成の例: SrcAlpha, One)
    // 通常の半透明合成なら SrcAlpha, InvSrcAlpha
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // 通常ブレンド
    psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // 両面描画
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

    // デプス設定 (Z書き込みはOFFにするのが一般的)
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.SampleDesc.Count = 1;

    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&graphicsPipelineState_));
    assert(SUCCEEDED(hr));
}

void ParticleManager::CreateInstancingResource()
{
    ID3D12Device* device = dxCommon_->GetDevice();

    // リソースのサイズ計算
    size_t sizeInBytes = sizeof(ParticleForGPU) * kNumMaxInstance;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = sizeInBytes;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    // バッファ作成
    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&instancingResource_));
    assert(SUCCEEDED(hr));

    // マッピング
    instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

    // SRV作成
    // SrvManagerから空きインデックスをもらう
    srvIndex_ = SrvManager::GetInstance()->Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = SrvManager::GetInstance()->GetCPUDescriptorHandle(srvIndex_);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBufferはUNKNOWN
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = kNumMaxInstance;
    srvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    device->CreateShaderResourceView(instancingResource_.Get(), &srvDesc, cpuHandle);
}