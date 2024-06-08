#pragma once
#include "Core/Core.h"
#include "FrameWork/Component/Component.h"
#include "Platform/Vulkan/Mesh/AssetLoader.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"

namespace ReEngine
{
    class StaticMeshComponent : public ComponentBase
    {
    public:
        StaticMeshComponent(Ref<VulkanMesh> Mesh);
        
    public:
        std::weak_ptr<VulkanMesh> Mesh;
        int32 PbrMaterialParamIndex = -1;

        std::string FileName;
    };
}
