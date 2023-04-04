#pragma once
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"

class VulkanIndexBuffer
{
public:

    ~VulkanIndexBuffer()
    {
        
    }

    void BindAndDraw(VkCommandBuffer CmdBuffer)
    {
        vkCmdBindIndexBuffer(CmdBuffer,Buffer->Buffer,0,IndexType);
        vkCmdDrawIndexed(CmdBuffer, IndexCount, 1, 0, 0, 0);
    }

    static VulkanIndexBuffer* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VulkanCommandBuffer* cmdBuffer, std::vector<uint16> indices, VkIndexType type = VK_INDEX_TYPE_UINT16);
    
public:
    VkDevice Device = VK_NULL_HANDLE;
    
    VulkanBuffer* Buffer = nullptr;
    
    int32 IndexCount = 0;
    VkIndexType	 IndexType = VK_INDEX_TYPE_UINT16;
    
private:
    VulkanIndexBuffer(){}
};
