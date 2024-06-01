#pragma once

#include "Core/Core.h"
#include "entt/include/entt.hpp"
#include "FrameWork/Scene.h"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "RenderGraph/SceneGraph.h"


namespace ReEngine
{
    class VulkanScene
    {
    public:
        VulkanScene();
        
        Ref<Entity> CreateEntity(const std::string& name,Ref<Entity> Parent = nullptr);
        
        Scene  mScene;
        
        std::vector<Ref<Entity>> EntityNodes;
    };    
}

