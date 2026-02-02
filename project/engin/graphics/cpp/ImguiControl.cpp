#include "ImguiControl.h"
#include "ImGuiManager.h"
#include "LightingMode.h"
#include "MaterialManager.h"
#include "MeshManager.h"

extern MeshManager meshManager;
extern MaterialManager materialManager;
extern int lightingMode;

void ShowControls()
{
    if(ImGui::CollapsingHeader("Mesh Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char* meshItems[] = { "Sphere", "Cube", "Plane" };
        int currentMesh = (int)meshManager.GetCurrentMeshType();
        if (ImGui::Combo("Mesh Type", &currentMesh, meshItems, IM_ARRAYSIZE(meshItems))) {
            meshManager.SetCurrentMeshType((MeshType)currentMesh);
        }

        // 各メッシュのトランスフォーム制御
        for (int i = 0; i < MeshType_Count; ++i) {
            ImGui::PushID(i);
            if (ImGui::TreeNode(meshItems[i])) {
                ImGui::DragFloat3("Scale", &meshManager.meshes[i].transform.scale.x, 0.01f);
                ImGui::DragFloat3("Rotation", &meshManager.meshes[i].transform.rotate.x, 0.01f);
                ImGui::DragFloat3("Translate", &meshManager.meshes[i].transform.translate.x, 0.01f);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    // マテリアル切り替え
    if (ImGui::CollapsingHeader("Material Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* matItems[] = { "Red", "Green", "Blue", "White" };
        int currentMat = (int)materialManager.GetCurrentMaterialIndex();
        if (ImGui::Combo("Material Color", &currentMat, matItems, IM_ARRAYSIZE(matItems))) {
            materialManager.SetCurrentMaterialIndex(currentMat);
        }
    }

    // ライティング切り替え
    if (ImGui::CollapsingHeader("Lighting Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* lightItems[] = { "None", "Lambert", "Half Lambert" };
        ImGui::Combo("Lighting Mode", &lightingMode, lightItems, IM_ARRAYSIZE(lightItems));
    }
}