#include "VulkanIndexBuffer.h"

VulkanIndexBuffer* VulkanIndexBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VulkanCommandBuffer* cmdBuffer,std::vector<uint16> indices, VkIndexType type)
{
    VulkanIndexBuffer* IndexBuffer = new VulkanIndexBuffer();
    IndexBuffer->IndexCount = indices.size();
    IndexBuffer->IndexType = type;
    
    VkDeviceSize IndexbufferSize = sizeof(indices[0]) * indices.size();

    VulkanBuffer* stagingIndexBuffer = VulkanBuffer::CreateBuffer(
            vulkanDevice,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            IndexbufferSize,(void*)indices.data());
    
   IndexBuffer->Buffer = VulkanBuffer::CreateBuffer(
            vulkanDevice,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ,
            IndexbufferSize);

    VulkanBuffer::TransferBuffer(vulkanDevice,cmdBuffer,stagingIndexBuffer,IndexBuffer->Buffer,IndexbufferSize);


    delete stagingIndexBuffer;
    return IndexBuffer;
}

