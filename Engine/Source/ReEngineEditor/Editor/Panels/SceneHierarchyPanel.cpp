#include "SceneHierarchyPanel.h"
#include "imgui.h"
#include "ECSFrameWork/Component/CameraComponent.h"

#include "ECSFrameWork/Component/Component.h"
#include "ECSFrameWork/Component/TransformComponent.h"
#include "glm/gtc/type_ptr.hpp"

namespace ReEngine
{
    SceneHierarchyPanel::SceneHierarchyPanel()
    {
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& Scene)
    {
        SetContext(Scene);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& Scene)
    {
        mContext = Scene;
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");
        mContext->mRegistry.each([&](auto entityID)
        {
           Entity entity{entityID,mContext.get()};
            DrawEntityNode(entity);
        });

        if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            mSelectContext = {};
        ImGui::End();

        ImGui::Begin("Properties");
        if (mSelectContext)
            DrawComponent(mSelectContext);

        ImGui::End();
        
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((mSelectContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        bool Opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
        if (ImGui::IsItemClicked())
        {
            mSelectContext = entity;
        }

        if(Opened)
        {
            // ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
            // bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
            ImGui::TreePop();
        }
    }

    void SceneHierarchyPanel::DrawComponent(Entity entity)
    {
        if(entity.HasComponent<TagComponent>())
        {
            auto& Tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), Tag.c_str());

            if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
            {
                Tag = std::string(buffer);
            }
            
            if (entity.HasComponent<TransformComponent>())
            {
                if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
                {
                    auto& Position = entity.GetComponent<TransformComponent>().Translation;
                    ImGui::DragFloat3("Position", glm::value_ptr(Position), 0.1f);

                    ImGui::TreePop();
                }
            }
        }


        if (entity.HasComponent<CameraComponnet>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(CameraComponnet).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
			{
				auto& cameraComponent = entity.GetComponent<CameraComponnet>();
				auto& camera = cameraComponent.Camera;

				ImGui::Checkbox("Primary", &cameraComponent.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &verticalFov))
						camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));

					float orthoNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &orthoNear))
						camera.SetPerspectiveNearClip(orthoNear);

					float orthoFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &orthoFar))
						camera.SetPerspectiveFarClip(orthoFar);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					;
					float orthoSize = camera.GetOrthoGraphicSize();
					if (ImGui::DragFloat("Size", &orthoSize,1,0,0,"%.3f"))
						camera.SetOrthoGraphicSize(orthoSize);

					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near", &orthoNear))
						camera.SetOrthographicNearClip(orthoNear);

					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far", &orthoFar))
						camera.SetOrthographicFarClip(orthoFar);

					ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);
				}


				ImGui::TreePop();
			}
		}
    }
}
