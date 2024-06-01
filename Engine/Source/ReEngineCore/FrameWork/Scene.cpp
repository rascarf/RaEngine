#include "Scene.h"
#include "Component/Component.h"
#include "Component/TransformComponent.h"
#include "glm/glm.hpp"
#include "Renderer/Renderer2D.h"

namespace ReEngine
{
    Entity::Entity(entt::entity handle, Scene* scene):mEntityHandle(handle),mScene(scene)
    {
    
    }

    Entity::Entity(Ref<Entity> ParentHandle, entt::entity handle, Scene* scene)
        :mEntityHandle(handle),mParent(ParentHandle),mScene(scene)
    {
    
    }

    glm::mat4 Entity::GetGloablTransform()
    {
        glm::mat4 WorldMatrix = mScene->mRegistry.get<TransformComponent>(mEntityHandle).GetTransform();
        if(mParent.expired())
        {
            WorldMatrix = mParent.lock()->GetGloablTransform() * WorldMatrix;
        }

        return WorldMatrix;
    }

    glm::mat4 Entity::GetLocalTransform()
    {
        glm::mat4 WorldMatrix = mScene->mRegistry.get<TransformComponent>(mEntityHandle).GetTransform();
        return WorldMatrix;
    }

    const std::string& Entity::GetName() const
    {
        return mScene->mRegistry.get<TagComponent>(mEntityHandle).Tag;
    }

    
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

    Ref<Entity> Scene::CreateEntity(const std::string& name)
    {
        Ref<Entity> entity = CreateRef<Entity>(mRegistry.create(),this);
        
        entity->AddComponent<TransformComponent>();
        entity->AddComponent<TagComponent>(name);
        return entity;
    }

    void Scene::DestroyEntity(Entity* entity)
    {
        mRegistry.destroy(*entity);
    }

    void Scene::OnRuntimeStart()
    {
    }

    void Scene::OnRuntimeStop()
    {
    }

    void Scene::OnUpdate(Timestep ts)
    {
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
    }

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
    {
    }

    void Scene::OnViewportResize(uint32_t Width, uint32_t Height)
    {
    }
    
}


