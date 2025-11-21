// --------------------------------------------------
// include
// --------------------------------------------------

#include "D3DResourceLeakChecker.h"
#include "DirectXCommon.h"
#include "DirectXTex.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "Logger.h"
#include "MakeAffine.h"
#include "ResourceObject.h"
#include "StringUtlity.h"
#include "Sprite.h"
#include "WinApp.h"
#include "d3dx12.h"
#include "SpriteCommon.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include <Windows.h>
#include <Xinput.h>
#include <cassert>
#include <cstdint>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <format>
#include <numbers>
#include <string>
#include <vector>
#include <wrl/client.h>

// --------------------------------------------------
// ライブラリのリンク
// --------------------------------------------------

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xinput.lib")

// --------------------------------------------------
// using declarations
// --------------------------------------------------

using namespace std::numbers;
using Microsoft::WRL::ComPtr;

// --------------------------------------------------
// 関数、構造体定義
// --------------------------------------------------

DirectX::ScratchImage LoadTexture(const std::string filePath)
{
    DirectX::ScratchImage image {};
    std::wstring filePathW = StringUtility::ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));
    DirectX::ScratchImage mipImages {};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 8, mipImages);
    assert(SUCCEEDED(hr));
    return mipImages;
}

ComPtr<ID3D12Resource> CreateTextureResourse(ID3D12Device* device, const DirectX::TexMetadata& metadata)
{
    D3D12_RESOURCE_DESC resourceDesc {};
    resourceDesc.Width = UINT(metadata.width);
    resourceDesc.Height = UINT(metadata.height);
    resourceDesc.MipLevels = UINT16(metadata.mipLevels);
    resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
    resourceDesc.Format = metadata.format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

    D3D12_HEAP_PROPERTIES heapProperties {};
    heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

    ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource));
    assert(SUCCEEDED(hr));
    return resource;
}

void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
{
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
        const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
        HRESULT hr = texture->WriteToSubresource(
            UINT(mipLevel),
            nullptr,
            img->pixels,
            UINT(img->rowPitch),
            UINT(img->slicePitch));
        assert(SUCCEEDED(hr));
    }
}

ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
{
    D3D12_HEAP_PROPERTIES heapProperties {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = sizeInBytes;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource));
    assert(SUCCEEDED(hr));

    return resource;
}

struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

struct Material {
    Vector4 color;
    int enableLighting;
    int shadingType; // 0: Lambert, 1: HalfLambert
    float padding[2];
    Matrix4x4 uvTransform;
};

struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

struct DirectionalLight {
    Vector4 color;
    Vector3 direction;
    float intensity;
};

ComPtr<ID3D12Resource> CreateBufferResouse(ID3D12Device* device, size_t sizeInBytes)
{
    D3D12_HEAP_PROPERTIES uploadHeapProperties {};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC vertexResourceDesc {};
    vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexResourceDesc.Width = sizeInBytes;
    vertexResourceDesc.Height = 1;
    vertexResourceDesc.DepthOrArraySize = 1;
    vertexResourceDesc.MipLevels = 1;
    vertexResourceDesc.SampleDesc.Count = 1;
    vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtr<ID3D12Resource> vertexResource = nullptr;
    HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&vertexResource));
    assert(SUCCEEDED(hr));
    return vertexResource;
}

// --------------------------------------------------
// メイン関数
// --------------------------------------------------

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    D3D12ResourceLeakChecker leakCheck;

    // ポインタ
    WinApp* winApp = nullptr;

    // windowsAPIの初期化
    winApp = new WinApp();
    winApp->Initialize();

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    DirectXCommon* dxCommon = nullptr;
    dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);

    HRESULT hr;

    ID3D12Device* device = dxCommon->GetDevice();

    // DirectXCommon初期化後
    SpriteCommon* spriteCommon = new SpriteCommon();
    spriteCommon->Initialize(dxCommon);

    Sprite* sprite = new Sprite();
    sprite->Initialize(spriteCommon);

    // 必要ならテクスチャハンドルを設定
    sprite->SetTextureHandle(textureSrvHandlesGPU[0]);

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature {};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRanges[1] = {};
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[4] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0;
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRanges;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRanges);
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 1;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    ID3DBlob* signatureBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    hr = device->CreateRootSignature(0,
        signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc {};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    D3D12_BLEND_DESC blendDesc {};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_RASTERIZER_DESC rasterizerDesc {};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    IDxcBlob* vertexShaderBlob = dxCommon->CompileShader(L"Resources/shaders/object3d/Object3dVS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    IDxcBlob* pixelShaderBlob = dxCommon->CompileShader(L"Resources/shaders/object3d/Object3dPS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc {};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.BlendState = blendDesc;
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
    hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
        IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));

    ComPtr<ID3D12Resource> wvpResource = CreateBufferResouse(device, sizeof(TransformationMatrix));
    TransformationMatrix* wvpData = nullptr;
    wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));

    ComPtr<ID3D12Resource> materialResource = CreateBufferResouse(device, sizeof(Vector4) * 3);
    Material* materialData = nullptr;
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->uvTransform = MakeIdentity4x4();

    const uint32_t kSubdivision = 32;
    const uint32_t kSphereVertexCount = kSubdivision * kSubdivision * 6;

    ComPtr<ID3D12Resource> vertexResource = CreateBufferResouse(device, sizeof(VertexData) * kSphereVertexCount);
    VertexData* vertexData = nullptr;
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    const float kRadius = 1.0f;
    const float kPi = std::numbers::pi_v<float>;
    const float kTwoPi = kPi * 2.0f;
    uint32_t vertexIdx = 0;
    for (uint32_t lat = 0; lat < kSubdivision; ++lat) {
        float lat0 = kPi * (float(lat) / kSubdivision - 0.5f);
        float lat1 = kPi * (float(lat + 1) / kSubdivision - 0.5f);
        for (uint32_t lon = 0; lon < kSubdivision; ++lon) {
            float lon0 = kTwoPi * float(lon) / kSubdivision;
            float lon1 = kTwoPi * float(lon + 1) / kSubdivision;
            Vector4 p00 = { kRadius * cos(lat0) * cos(lon0), kRadius * sin(lat0), kRadius * cos(lat0) * sin(lon0), 1.0f };
            Vector4 p01 = { kRadius * cos(lat0) * cos(lon1), kRadius * sin(lat0), kRadius * cos(lat0) * sin(lon1), 1.0f };
            Vector4 p10 = { kRadius * cos(lat1) * cos(lon0), kRadius * sin(lat1), kRadius * cos(lat1) * sin(lon0), 1.0f };
            Vector4 p11 = { kRadius * cos(lat1) * cos(lon1), kRadius * sin(lat1), kRadius * cos(lat1) * sin(lon1), 1.0f };
            Vector2 uv00 = { float(lon) / kSubdivision, 1.0f - float(lat) / kSubdivision };
            Vector2 uv01 = { float(lon + 1) / kSubdivision, 1.0f - float(lat) / kSubdivision };
            Vector2 uv10 = { float(lon) / kSubdivision, 1.0f - float(lat + 1) / kSubdivision };
            Vector2 uv11 = { float(lon + 1) / kSubdivision, 1.0f - float(lat + 1) / kSubdivision };
            vertexData[vertexIdx++] = { p00, uv00 };
            vertexData[vertexIdx++] = { p10, uv10 };
            vertexData[vertexIdx++] = { p11, uv11 };
            vertexData[vertexIdx++] = { p00, uv00 };
            vertexData[vertexIdx++] = { p11, uv11 };
            vertexData[vertexIdx++] = { p01, uv01 };
        }
    }
    for (uint32_t i = 0; i < vertexIdx; ++i) {
        vertexData[i].normal.x = vertexData[i].position.x;
        vertexData[i].normal.y = vertexData[i].position.y;
        vertexData[i].normal.z = vertexData[i].position.z;
    }

    ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
    DirectionalLight* directionalLightData = nullptr;
    directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
    directionalLightData->intensity = 1.0f;

    ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material));
    vertexResource->Unmap(0, nullptr);
    Material* materialDataSprite = nullptr;
    materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
    materialDataSprite->enableLighting = false;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(VertexData) * kSphereVertexCount;
    vertexBufferView.StrideInBytes = sizeof(VertexData);

    // Transform変数の定義
    Transform transform { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

    std::vector<std::string> textureFiles = { "Resources/uvChecker.png", "Resources/monsterBall.png" };
    std::vector<ComPtr<ID3D12Resource>> textureResources;
    std::vector<DirectX::ScratchImage> mipImagesList;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandlesCPU;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandlesGPU;

    // dxCommonからSRVヒープを取得
    ID3D12DescriptorHeap* srvDescriptorHeap = dxCommon->GetSrvDescriptorHeap();
    UINT srvIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

    srvHandleCPU.ptr += srvIncrement;
    srvHandleGPU.ptr += srvIncrement;

    for (size_t i = 0; i < textureFiles.size(); ++i) {
        mipImagesList.push_back(LoadTexture(textureFiles[i]));
        const DirectX::TexMetadata& metadata = mipImagesList.back().GetMetadata();
        ComPtr<ID3D12Resource> texRes = CreateTextureResourse(device, metadata);
        UploadTextureData(texRes.Get(), mipImagesList.back());
        textureResources.push_back(texRes);
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
        srvDesc.Format = metadata.format;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
        device->CreateShaderResourceView(texRes.Get(), &srvDesc, srvHandleCPU);
        textureSrvHandlesCPU.push_back(srvHandleCPU);
        textureSrvHandlesGPU.push_back(srvHandleGPU);
        srvHandleCPU.ptr += srvIncrement;
        srvHandleGPU.ptr += srvIncrement;
    }

    // ImGuiの初期化
    ImGuiManager* imguiManager = new ImGuiManager();
    imguiManager->Initialize(winApp, dxCommon);

    ImGuiIO& io = ImGui::GetIO();

    static int sphereTextureIndex = 0;

    Input* input = nullptr;
    input = new Input();
    input->Initialize(winApp);

    // --------------------------------------------------
    // メインループ
    // --------------------------------------------------

    while (true) {
        if (winApp->ProcessMessage()) {
            break;
        } else {

            imguiManager->Begin();

            dxCommon->PreDraw();

            // コマンドリストの取得
            ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

            // 入力の更新
            input->Update();

            const float kMoveSpeed = 0.1f;
            if (input->PushKey(DIK_W))
                transform.translate.y += kMoveSpeed;
            if (input->PushKey(DIK_S))
                transform.translate.y -= kMoveSpeed;
            if (input->PushKey(DIK_A))
                transform.translate.x -= kMoveSpeed;
            if (input->PushKey(DIK_D))
                transform.translate.x += kMoveSpeed;

            XINPUT_STATE state;
            ZeroMemory(&state, sizeof(XINPUT_STATE));
            DWORD dwResult = XInputGetState(0, &state);
            if (dwResult == ERROR_SUCCESS) {
                io.AddKeyEvent(ImGuiKey_GamepadDpadUp, (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0);
                io.AddKeyEvent(ImGuiKey_GamepadDpadDown, (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
                io.AddKeyEvent(ImGuiKey_GamepadDpadLeft, (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
                io.AddKeyEvent(ImGuiKey_GamepadDpadRight, (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
                io.AddKeyEvent(ImGuiKey_GamepadFaceDown, (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0);
                io.AddKeyEvent(ImGuiKey_GamepadFaceRight, (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0);
                float lx = state.Gamepad.sThumbLX / 32767.0f;
                float ly = state.Gamepad.sThumbLY / 32767.0f;
                io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickLeft, lx < -0.3f, lx);
                io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickRight, lx > 0.3f, lx);
                io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickUp, ly > 0.3f, ly);
                io.AddKeyAnalogEvent(ImGuiKey_GamepadLStickDown, ly < -0.3f, ly);
            }

            static int sphereShadingType = 0;
            static bool sphereEnableLighting = false;

            ImGui::ShowDemoWindow();
            ImGui::Begin("Main Control");
            ImGui::Text("Sprite");
            ImGui::DragFloat3("Sprite Position", &sprite->GetTranslate().x);
            ImGui::DragFloat3("Sprite Rotation", &sprite->GetRotate().x);
            ImGui::DragFloat3("Sprite Scale", &sprite->GetScale().x);
            ImGui::Separator();
            ImGui::Text("Sphere");
            ImGui::DragFloat3("Sphere Position", &transform.translate.x, 0.1f);
            ImGui::DragFloat3("Sphere Rotation", &transform.rotate.x, 0.01f);
            ImGui::DragFloat3("Sphere Scale", &transform.scale.x, 0.01f, 0.1f, 10.0f);
            ImGui::ColorEdit3("Sphere Color", &materialDataSprite->color.x);
            ImGui::Combo("Sphere Texture", &sphereTextureIndex, "texture1\0texture2\0");
            ImGui::Checkbox("Enable Lighting", &sphereEnableLighting);
            const char* shadingTypes[] = { "Lambert", "HalfLambert" };
            ImGui::Combo("Sphere Shading", &sphereShadingType, shadingTypes, IM_ARRAYSIZE(shadingTypes));
            ImGui::Separator();
            ImGui::Text("Light");
            ImGui::DragFloat3("Light Direction", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);
            ImGui::End();

            materialDataSprite->enableLighting = sphereEnableLighting;
            materialDataSprite->shadingType = sphereShadingType;

            {
                float& x = directionalLightData->direction.x;
                float& y = directionalLightData->direction.y;
                float& z = directionalLightData->direction.z;
                float len = sqrtf(x * x + y * y + z * z);
                if (len > 0.0001f) {
                    x /= len;
                    y /= len;
                    z /= len;
                }
            }

            ImGui::Render();

            // SRV用ヒープの設定 (再設定)
            ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
            commandList->SetDescriptorHeaps(1, descriptorHeaps);

            transform.rotate.y += 0.00f;

            Transform cameraTransform { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -10.0f } };
            Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
            Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
            Matrix4x4 viewMatrix = Inverse(cameraMatrix);
            Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
            Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
            wvpData->WVP = worldViewProjectionMatrix;
            wvpData->World = worldMatrix;

            Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
            uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
            uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
            materialData->uvTransform = uvTransformMatrix;

            Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
            Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
            Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
            Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
            *transformationMatrixDataSprite = worldViewProjectionMatrixSprite;

            // 描画コマンド発行
            commandList->SetGraphicsRootSignature(rootSignature.Get());
            commandList->SetPipelineState(graphicsPipelineState.Get());
            commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
            commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
            commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandlesGPU[sphereTextureIndex]);
            commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
            // commandList->DrawInstanced(kSphereVertexCount, 1, 0, 0);

            // Spriteの更新・描画
            sprite->Update();
            sprite->Draw();

            imguiManager->End();
            imguiManager->Draw(dxCommon);

            // 描画後処理
            dxCommon->PostDraw();
        }
    }

    Logger::Log("Hello, DirectX!\n");
    Logger::Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
    ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        D3D12_MESSAGE_ID denyIds[] = {
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter {};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        infoQueue->PushStorageFilter(&filter);
    }
#endif

    imguiManager->Finalize();
    winApp->Finalize();

    delete sprite;
    delete spriteCommon;
    delete input;
    delete imguiManager;
    delete dxCommon;
    delete winApp;

    return 0;
}