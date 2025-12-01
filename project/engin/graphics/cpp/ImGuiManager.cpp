#include "ImGuiManager.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include <SrvManager.h>

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon)
{
    // ImGui生成
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Win32初期化
    ImGui_ImplWin32_Init(winApp->GetHwnd());

    // DirectX12初期化
    uint32_t index = SrvManager::GetInstance()->Allocate();

    ImGui_ImplDX12_Init(
        dxCommon->GetDevice(),
        dxCommon->GetBufferCount(),
        dxCommon->GetBackBufferFormat(),
        SrvManager::GetInstance()->GetSrvDescriptorHeap(), // ヒープを渡す
        SrvManager::GetInstance()->GetCPUDescriptorHandle(index), // 確保したCPUハンドル
        SrvManager::GetInstance()->GetGPUDescriptorHandle(index) // 確保したGPUハンドル
    );
}

void ImGuiManager::Begin()
{
    // フレーム開始
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::End()
{
    // 内部コマンド生成
    ImGui::Render();
}

void ImGuiManager::Draw(DirectXCommon* dxCommon)
{
    // コマンドリストをdxCommonから取得する
    ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

    // 描画コマンドを発行
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiManager::Finalize()
{
    // 終了処理
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}