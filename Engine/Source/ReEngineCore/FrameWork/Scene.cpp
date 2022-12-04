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
namespace ReEngine
{
    
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    Ref<Scene> Scene::Copy(Ref<Scene> Other)
    {
        Ref<Scene> newScene = CreateRef<Scene>();
        
        return newScene;
    }

    void Scene::ChangeDimMode()
    {
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = {mRegistry.create(),this};
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<TagComponent>(name);
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        mRegistry.destroy(entity);
    }

    void Scene::OnRuntimeStart()
    {
    }

    void Scene::OnRuntimeStop()
    {
    }

    void Scene::OnUpdate(Timestep ts)
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
            
            Renderer2D::EndScene();
        }
    }
    
    Entity Scene::GetPrimaryEntity(Entity entity)
    {
        return entity;
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
    }

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
    {
    }

    void Scene::OnViewportResize(uint32_t Width, uint32_t Height)
    {
        // Resize Our Non Fixed Ratio Camera
        auto view = mRegistry.view<CameraComponnet>();
        for(auto entity:view)
        {
            auto& cameraCop = view.get<CameraComponnet>(entity);
            if(!cameraCop.FixedAspectRatio)
            {
                cameraCop.Camera.SetViewportSize(Width,Height);
            }
        }
    }

    // template<Component... C>
    // static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
    // {
    // ([&]()
    //     {
    //         auto view = src.view<C>();
    //         for (auto srcEntity : view)
    //         {
    //             entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);
    //
    //             auto& srcComponent = src.get<C>(srcEntity);
    //             dst.emplace_or_replace<C>(dstEntity, srcComponent);
    //         }
    //     }(), ...);
    // }
    //
    // template<Component... C>
    // static void CopyComponent(ComponentGroup<C...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
    // {
    //     CopyComponent<C...>(dst, src, enttMap);
    // }
    //
    // template<Component... C>
    // static void CopyComponentIfExists(Entity dst, Entity src)
    // {
    //     ([&]()
    //         {
    //             if (src.HasComponent<C>())
    //                 dst.AddOrReplaceComponent<C>(src.GetComponent<C>());
    //         }(), ...);
    // }
    //
    // template<Component... C>
    // static void CopyComponentIfExists(ComponentGroup<C...>, Entity dst, Entity src)
    // {
    //     CopyComponentIfExists<C...>(dst, src);
    // }

    void Scene::DuplicateEntity(Entity entity)
    {
        Entity newEntity = CreateEntity(entity.GetName());
    
    }
}


