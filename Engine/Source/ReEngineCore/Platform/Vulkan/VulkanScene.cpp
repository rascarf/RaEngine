#include "VulkanScene.h"

namespace ReEngine
{
    VulkanScene::VulkanScene()
    {
        
    }

    Ref<Entity> VulkanScene::CreateEntity(const std::string& name,Ref<Entity> Parent)
    {
        Ref<Entity> CreateEntity =  mScene.CreateEntity(name);
        EntityNodes.emplace_back(CreateEntity);

        if(Parent)
        {
            CreateEntity->mParent = Parent;
            
            Parent->mChildren.emplace_back(CreateEntity);
        }

        return CreateEntity;
    }
}


