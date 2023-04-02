#pragma once
#include "VulkanCommonDefine.h"
#include "VulkanDevice.h"

class VulkanCommandBuffer
{
public:
    ~VulkanCommandBuffer();

public:
    void Begin();
    void End();

    void Submit(VkSemaphore* SignalSemaphore = nullptr);
    static VulkanCommandBuffer* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
public:
    VkCommandBuffer						CmdBuffer = VK_NULL_HANDLE;
    VkFence								Fence = VK_NULL_HANDLE;
    VkCommandPool						CommandPool = VK_NULL_HANDLE;
    std::shared_ptr<VulkanDevice>		m_VulkanDevice = nullptr;
    std::vector<VkPipelineStageFlags>	WaitFlags;
    std::vector<VkSemaphore>			WaitSemaphores;

    bool								IsBegun;
private:
    VulkanCommandBuffer(){}
};
