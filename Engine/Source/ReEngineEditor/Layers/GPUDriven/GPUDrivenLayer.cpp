#include "GPUDrivenLayer.h"
#include "Platform/Vulkan/VulkanContext.h"

void GPUDrivenLayer::OnNativeLoadAsset()
{
    const auto device = VkContext->Instance->GetDevice();
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);

    Ref<Entity> entity = m_Scene->CreateEntity("Root");
    Ref<Entity> Child = m_Scene->CreateEntity("Child",entity);
    
    // std::vector<VertexAttribute> VertexAttribute = {VertexAttribute::VA_Position,VertexAttribute::VA_UV0,VertexAttribute::VA_Normal};
    // SceneModel = VulkanModel::LoadFromFile("Assets/Mesh/bunny.obj",device,cmdBuffer,VertexAttribute);
}

void GPUDrivenLayer::OnNativeUpdate(Timestep ts)
{
}

void GPUDrivenLayer::OnNativeDeInit()
{
}
