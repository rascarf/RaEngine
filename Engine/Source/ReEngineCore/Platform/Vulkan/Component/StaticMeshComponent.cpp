#include "StaticMeshComponent.h"

#include "Platform/Vulkan/VulkanScene.h"


StaticMeshComponent::StaticMeshComponent(Ref<VulkanMesh> InMesh)
{
    Mesh = InMesh;
}


