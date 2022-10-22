#pragma once
#include "Camera/EditorCamera.h"
#include "Camera/OrthographicCamera.h"
#include "Core/PCH.h"
#include "Core/Core.h"

#include "ThirdParty/entt/include/entt.hpp"
namespace ReEngine
{
    class Entity;
    
    class Scene
    {
    public:
        Scene();
        ~Scene();

        static Ref<Scene> Copy(Ref<Scene> Other);
        void ChangeDimMode();
        
        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        void OnRuntimeStart();
        void OnRuntimeStop();
        void OnUpdate(Timestep ts,OrthographicCamera camera);

        void OnUpdateRuntime(Timestep ts);
        void OnUpdateEditor(Timestep ts,EditorCamera& camera);
        void OnViewportResize(uint32_t Width,uint32_t Height);

        void DuplicateEntity(Entity entity);

        Entity GetPrimaryEntity(Entity entity);

        template<typename... Components>
        auto GetAllEntitiesWith()
        {
            return mRegistry.view<Components...>();
        }
    public:
        entt::registry mRegistry; //保存Component的Data和ID
        
    private:
        friend class Entity;

        
        template<typename T>
        void OnComponentAdded(ReEngine::Entity entity,T& component){};

    private:
        // std::vector<Scope<class System>> mSystems;
        friend class Entity;
        // friend class SceneSerializer;
        // friend class SceneHierarchyPanel;
    };


}


