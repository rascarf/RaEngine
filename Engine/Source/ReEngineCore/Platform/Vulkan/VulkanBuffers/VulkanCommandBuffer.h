#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanDevice.h"

class VulkanCommandBuffer
{
public:
    ~VulkanCommandBuffer();

public:
    void Begin();
    void End();

    void Submit(VkSemaphore* SignalSemaphore = nullptr);

    //分配使用单次的
    static Ref<VulkanCommandBuffer> Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,std::shared_ptr<VulkanQueue> queue = nullptr);
public:
    VkCommandBuffer						CmdBuffer = VK_NULL_HANDLE;
    VkFence								Fence = VK_NULL_HANDLE;
    VkCommandPool						CommandPool = VK_NULL_HANDLE;
    std::weak_ptr<VulkanDevice>		m_VulkanDevice;
    std::vector<VkPipelineStageFlags>	WaitFlags;
    std::vector<VkSemaphore>			WaitSemaphores;

    Ref<VulkanQueue>                    queue = nullptr;

    bool								IsBegun;

};
