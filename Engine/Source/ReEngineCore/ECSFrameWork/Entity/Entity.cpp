#include "Entity.h"

#include "ECSFrameWork/Component/Component.h"

namespace ReEngine
{
    Entity::Entity(entt::entity handle, Scene* scene):mEntityHandle(handle),mScene(scene)
    {
        
    }

    const std::string& Entity::GetName() const
    {
        return mScene->mRegistry.get<TagComponent>(mEntityHandle).Tag;
    }

}
