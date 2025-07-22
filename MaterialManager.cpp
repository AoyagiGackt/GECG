#include "MaterialManager.h"

MaterialManager::MaterialManager()
    : currentMaterialIndex_(0)
{
    InitMaterials();
}
void MaterialManager::SetCurrentMaterialIndex(size_t index) { currentMaterialIndex_ = index; }
size_t MaterialManager::GetCurrentMaterialIndex() const { return currentMaterialIndex_; }
Material& MaterialManager::GetCurrentMaterial() { return materials[currentMaterialIndex_]; }

void MaterialManager::InitMaterials()
{
    // ��: �ԁA�΁A�A�J�X�^��
    materials.push_back({ { 1, 0, 0, 1 }, 1, { 0, 0, 0 }, MakeIdentity4x4() }); // Red
    materials.push_back({ { 0, 1, 0, 1 }, 1, { 0, 0, 0 }, MakeIdentity4x4() }); // Green
    materials.push_back({ { 0, 0, 1, 1 }, 1, { 0, 0, 0 }, MakeIdentity4x4() }); // Blue
    materials.push_back({ { 1, 1, 1, 1 }, 1, { 0, 0, 0 }, MakeIdentity4x4() }); // White/Custom
}