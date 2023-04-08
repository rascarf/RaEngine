#pragma once
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanDevice.h"

class VulkanIndexBuffer
{
public:

    ~VulkanIndexBuffer()
    {
        Buffer.reset();
        Buffer = nullptr;
    }

    void BindAndDraw(VkCommandBuffer CmdBuffer)
    {
        vkCmdBindIndexBuffer(CmdBuffer,Buffer->Buffer,0,IndexType);
        vkCmdDrawIndexed(CmdBuffer, IndexCount, 1, 0, 0, 0);
    }

    static Ref<VulkanIndexBuffer> Create(std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer, std::vector<uint16> indices, VkIndexType type = VK_INDEX_TYPE_UINT16);
    
public:
    VkDevice Device = VK_NULL_HANDLE;
    
    Ref<VulkanBuffer> Buffer = nullptr;
    
    int32 IndexCount = 0;
    VkIndexType	 IndexType = VK_INDEX_TYPE_UINT16;
};
