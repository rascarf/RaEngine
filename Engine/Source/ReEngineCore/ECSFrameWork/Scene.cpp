#include "Scene.h"

#include "imgui.h"
#include "Component/Component.h"
#include "Component/CameraComponent.h"
#include "Component/SpriteRenderComponent.h"
#include "Component/TransformComponent.h"
#include "Entity/Entity.h"
#include "glm/glm.hpp"
#include "Renderer/Renderer2D.h"
#include "Renderer/RHI/RenderCommand.h"

ReEngine::Scene::Scene()
{
}

ReEngine::Scene::~Scene()
{
}

ReEngine::Ref<ReEngine::Scene> ReEngine::Scene::Copy(Ref<Scene> Other)
{
    Ref<Scene> newScene = CreateRef<Scene>();
    
    return newScene;
}

void ReEngine::Scene::ChangeDimMode()
{
}

ReEngine::Entity ReEngine::Scene::CreateEntity(const std::string& name)
{
    Entity entity = {mRegistry.create(),this};
    entity.AddComponent<TransformComponent>();
    // entity.AddComponent<TagComponent>(name);
    return entity;
}

void ReEngine::Scene::DestroyEntity(Entity entity)
{
    mRegistry.destroy(entity);
}

void ReEngine::Scene::OnRuntimeStart()
{
}

void ReEngine::Scene::OnRuntimeStop()
{
}

void ReEngine::Scene::OnUpdate(Timestep ts,OrthographicCamera camera)
{
    Camera* mainCamera = nullptr;
    glm::mat4 CameraTransform;
    auto group = mRegistry.view<TransformComponent,CameraComponnet>();
    for(auto Entity:group)
    {
        auto [transformComponent,cameraComponent] = group.get<TransformComponent,CameraComponnet>(Entity);
        if(cameraComponent.Primary)
        {
            mainCamera = &cameraComponent.Camera;
            CameraTransform = transformComponent.GetTransform();
            break;
        }
    }

    if(mainCamera)
    {
        Renderer2D::BeginScene(*mainCamera,CameraTransform);
        
        auto SpriteRenderGroup = mRegistry.view<TransformComponent,SpriteRenderComponent>();
        for(auto Entity:SpriteRenderGroup)
        {
            auto [transformComponent,spriteRenderComponent] = SpriteRenderGroup.get<TransformComponent,SpriteRenderComponent>(Entity);
            Renderer2D::DrawQuad(transformComponent.GetTransform(),spriteRenderComponent.Color);
        }
    
        // Renderer2D::DrawQuad(glm::vec3(0,0,1),glm::vec2(1,1),glm::vec4(0.5,1.0,1.0,1.0));
        
        Renderer2D::EndScene();
    }
}

ReEngine::Entity ReEngine::Scene::GetPrimaryEntity(Entity entity)
{
    return entity;
}

void ReEngine::Scene::OnUpdateRuntime(Timestep ts)
{
}

void ReEngine::Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
{
}

void ReEngine::Scene::OnViewportResize(uint32_t Width, uint32_t Height)
{
}

void ReEngine::Scene::DuplicateEntity(Entity entity)
{
}


