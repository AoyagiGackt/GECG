#include "Framework.h"
#include "D3DResourceLeakChecker.h"
#include "SrvManager.h"

static D3D12ResourceLeakChecker leakCheck;

void Framework::Run()
{
    Initialize();
    while (true) {
        Update();
        if (IsEndRequest())
            break;
        Draw();
    }
    Finalize();
}

void Framework::Initialize()
{
    winApp_ = new WinApp();
    winApp_->Initialize();
    dxCommon_ = new DirectXCommon();
    dxCommon_->Initialize(winApp_);
    SrvManager::GetInstance()->Initialize(dxCommon_);
    input_ = new Input();
    input_->Initialize(winApp_);
    audio_ = new Audio();
    audio_->Initialize();
    imguiManager_ = new ImGuiManager();
    imguiManager_->Initialize(winApp_, dxCommon_);
}

void Framework::Update()
{
    input_->Update();
    imguiManager_->Begin();
}

void Framework::Finalize()
{
    imguiManager_->Finalize();
    audio_->Finalize();
    delete imguiManager_;
    delete audio_;
    delete input_;
    delete dxCommon_;
    delete winApp_;
}