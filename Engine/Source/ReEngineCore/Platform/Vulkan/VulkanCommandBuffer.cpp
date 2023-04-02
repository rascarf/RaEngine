#include "VulkanCommandBuffer.h"

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    
}

//使用的时候必须显示地进行Begin和End
void VulkanCommandBuffer::Begin()
{
    if (IsBegun)
    {
        return;
    }
    
    IsBegun = true;

    VkCommandBufferBeginInfo cmdBufBeginInfo;
    ZeroVulkanStruct(cmdBufBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(CmdBuffer, &cmdBufBeginInfo);
}

void VulkanCommandBuffer::End()
{
    if (!IsBegun)
    {
        return;
    }

    IsBegun = false;
    vkEndCommandBuffer(CmdBuffer);
}


void VulkanCommandBuffer::Submit(VkSemaphore* SignalSemaphore)
{
    End();
    
    VkSubmitInfo submitInfo;
    ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &CmdBuffer;
    submitInfo.signalSemaphoreCount = SignalSemaphore ? 1 : 0;
    submitInfo.pSignalSemaphores    = SignalSemaphore;
		
    if (WaitFlags.size() > 0) 
    {
        submitInfo.waitSemaphoreCount = WaitSemaphores.size();
        submitInfo.pWaitSemaphores    = WaitSemaphores.data();
        submitInfo.pWaitDstStageMask  = WaitFlags.data();
    }

    vkResetFences(m_VulkanDevice->GetInstanceHandle(), 1, &Fence);
    vkQueueSubmit(m_VulkanDevice->GetGraphicsQueue()->GetHandle(), 1, &submitInfo, Fence);
    vkWaitForFences(m_VulkanDevice->GetInstanceHandle(), 1, &Fence, true, ((uint64)	0xffffffffffffffff));
}

VulkanCommandBuffer* VulkanCommandBuffer::Create(std::shared_ptr<::VulkanDevice> vulkanDevice,VkCommandPool commandPool, VkCommandBufferLevel level)
{
    VkDevice device = vulkanDevice->GetInstanceHandle();

    VulkanCommandBuffer* cmdBuffer = new VulkanCommandBuffer();
    cmdBuffer->m_VulkanDevice = vulkanDevice;
    cmdBuffer->CommandPool  = commandPool;
    cmdBuffer->IsBegun      = false;

    VkCommandBufferAllocateInfo cmdBufferAllocateInfo;
    ZeroVulkanStruct(cmdBufferAllocateInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    cmdBufferAllocateInfo.commandPool = commandPool;
    cmdBufferAllocateInfo.level       = level;
    cmdBufferAllocateInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &cmdBufferAllocateInfo, &(cmdBuffer->CmdBuffer));

    VkFenceCreateInfo fenceCreateInfo;
    ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
    fenceCreateInfo.flags = 0;
    vkCreateFence(device, &fenceCreateInfo, nullptr, &(cmdBuffer->Fence));

    return cmdBuffer;
}
