#include "VulkanIndexBuffer.h"

Ref<VulkanIndexBuffer> VulkanIndexBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer,std::vector<uint16> indices, VkIndexType type)
{
    Ref<VulkanIndexBuffer> IndexBuffer = CreateRef<VulkanIndexBuffer>();

    IndexBuffer->IndexCount = indices.size();
    IndexBuffer->IndexType = type;
    
    VkDeviceSize IndexbufferSize = sizeof(indices[0]) * indices.size();

    auto stagingIndexBuffer = VulkanBuffer::CreateBuffer(
            vulkanDevice,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            IndexbufferSize,(void*)indices.data());
    
   IndexBuffer->Buffer = VulkanBuffer::CreateBuffer(
            vulkanDevice,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            IndexbufferSize);

    VulkanBuffer::TransferBuffer(vulkanDevice,cmdBuffer,stagingIndexBuffer,IndexBuffer->Buffer,IndexbufferSize);
    
    return IndexBuffer;
}

