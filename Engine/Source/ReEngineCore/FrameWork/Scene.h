#pragma once
#include "Camera/EditorCamera.h"
#include "Component/Component.h"
#include "Core/PCH.h"
#include "Core/Core.h"
#include "entt.hpp"

namespace ReEngine
{
    class Entity;
    
    class Scene
    {
    public:
        Scene();
        virtual ~Scene();

        static Ref<Scene> Copy(Ref<Scene> Other);
        virtual void ChangeDimMode();
        
        virtual Ref<class Entity> CreateEntity(const std::string& name = std::string());
        virtual void DestroyEntity(Entity* entity);

        virtual void OnRuntimeStart();
        virtual void OnRuntimeStop();
        virtual void OnUpdate(Timestep ts);

        virtual void OnUpdateRuntime(Timestep ts);
        virtual void OnUpdateEditor(Timestep ts,EditorCamera& camera);
        virtual void OnViewportResize(uint32_t Width,uint32_t Height);

        template<typename... Components>
        auto GetAllEntitiesWith()
        {
            return mRegistry.view<Components...>();
        }

        template<typename T>
        void OnComponentAdded(Entity* entity,T& component){};

        template<>
        void OnComponentAdded(Entity* entity,TagComponent& component)
        {
            // RE_INFO("{0}",component.Tag);
        }
        
    public:
        entt::registry mRegistry; //保存Component的Data和ID
        
        private:
        friend class Entity;
    
    private:
        friend class Entity;
        friend class SceneSerializer;
        // std::vector<Scope<class System>> mSystems;
        // friend class SceneHierarchyPanel;
    };
    
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, class Scene* scene);
        Entity(const Entity& other) = default;
        Entity(Ref<Entity> ParentHandle,entt::entity handle,Scene* scene);

        template<typename T,typename... Args>
        T& AddComponent(Args&&... args)
        {
            RE_CORE_ASSERT(!HasComponent<T>(),"Entity already has component!");
            T& Component = mScene->mRegistry.emplace<T>(mEntityHandle,std::forward<Args>(args)...);
            mScene->OnComponentAdded<T>(this,Component);
            return Component;
        }

        template<typename T,typename... Args>
        T& AddOrReplaceComponent(Args&&... args)
        {
            T& Component = mScene->mRegistry.emplace_or_replace<T>(mEntityHandle,std::forward<Args>(args)...);
            mScene->OnComponentAdded<T>(this,Component);
            return Component;
        }

        template<typename T>
        T& GetComponent()
        {
            RE_CORE_ASSERT(HasComponent<T>(),"Entity does not have component!");
            return mScene->mRegistry.get<T>(mEntityHandle);
        }

        template<typename... T>
        std::tuple<T*...>GetComponents()
        {
            HE_CORE_ASSERT((HasComponent<T>() && ...), "Entity does not have component!");
            return std::make_tuple(((const T* const)&mScene->mRegistry.get<T>(mEntityHandle))...);
        }

        template<typename T>
        bool HasComponent()
        {
            return mScene->mRegistry.all_of<T>(mEntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            mScene->mRegistry.remove<T>(mEntityHandle);
        }

        Ref<Entity> GetParent()
        {
            if(mParent.expired())
            {
                return mParent.lock();
            }
            
            return nullptr;
        }

        glm::mat4 GetGloablTransform();
        glm::mat4 GetLocalTransform();
        const std::string& GetName() const;

        operator bool() const{return mEntityHandle != entt::null; }
        operator entt::entity() const {return mEntityHandle; }
        operator uint32_t() const {return (uint32_t)mEntityHandle; }
        
        bool operator==(const Entity& other) const
        {
            return mEntityHandle == other.mEntityHandle && mScene == other.mScene;
        }

        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }

    public:
        entt::entity mEntityHandle{entt::null};
        
        std::weak_ptr<Entity> mParent;
        std::vector<std::weak_ptr<Entity>> mChildren;
        
       Scene* mScene; //TODO 不需要所有权
    };
    
    


}


