#pragma once
#include "Core/Core.h"
#include "Core/PCH.h"
#include "FrameWork/Scene.h"

namespace ReEngine
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene);
        Entity(const Entity& other) = default;

        template<typename T,typename... Args>
        T& AddComponent(Args&&... args)
        {
            RE_CORE_ASSERT(!HasComponent<T>(),"Entity already has component!");
            T& Component = mScene->mRegistry.emplace<T>(mEntityHandle,std::forward<Args>(args)...);
            mScene->OnComponentAdded<T>(*this,Component);
            return Component;
        }

        template<typename T,typename... Args>
        T& AddOrReplaceComponent(Args&&... args)
        {
            T& Component = mScene->mRegistry.emplace_or_replace<T>(mEntityHandle,std::forward<Args>(args)...);
            mScene->OnComponentAdded<T>(*this,Component);
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

        operator bool() const{return mEntityHandle != entt::null; }
        operator entt::entity() const {return mEntityHandle; }
        operator uint32_t() const {return (uint32_t)mEntityHandle; }

        const std::string& GetName() const;

        bool operator==(const Entity& other) const
        {
            return mEntityHandle == other.mEntityHandle && mScene == other.mScene;
        }

        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }

    private:
        entt::entity mEntityHandle{entt::null};
        Scene* mScene = nullptr; //TODO 不需要所有权
    };
}
