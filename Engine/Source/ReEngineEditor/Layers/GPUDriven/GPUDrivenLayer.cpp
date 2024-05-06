#include "GPUDrivenLayer.h"
#include "GPUDrivenLayer.h"

#include "Platform/Vulkan/VulkanContext.h"

void GPUDrivenLayer::OnNativeLoadAsset()
{
    const auto device = VkContext->Instance->GetDevice();
    Scene.LoadglTFModel(device,VkContext->CommandPool,"Assets/Mesh/diorama/Diorama.glb");
    
}

void GPUDrivenLayer::OnNativeUpdate(Timestep ts)
{
    
}
