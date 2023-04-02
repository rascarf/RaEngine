﻿#include "VulkanCommandPool.h"

#include "VulkanCommandBuffer.h"
#include "Core/Application.h"
#include "VulkanContext.h"

namespace ReEngine
{
void VulkanCommandPool::Init(VulkanContext* Context)
{
    const auto Instance = Context->GetVulkanInstance();
    const auto vulkanDevice = Instance->GetDevice();
    m_Device   = Instance->GetDevice()->GetInstanceHandle();
    m_VulkanDevice  = Instance->GetDevice();
    m_SwapChain = Instance->GetSwapChain();
        
    m_GfxQueue      = vulkanDevice->GetGraphicsQueue()->GetHandle();
    m_PresentQueue  = vulkanDevice->GetPresentQueue()->GetHandle();
    m_FrameWidth    = Instance->GetWidth();
    m_FrameHeight   = Instance->GetHeight();
        
    CreateFences();
    CreateCommandBuffers();
    CreatePipelineCache();
    CreateDefaultRes();
}

void VulkanCommandPool::Present(int backBufferIndex)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask    = &m_WaitStageMask;
    submitInfo.pWaitSemaphores      = &m_PresentComplete;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pSignalSemaphores    = &m_RenderComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers      = &(m_CommandBuffers[backBufferIndex]);
    submitInfo.commandBufferCount   = 1;

    vkResetFences(m_Device, 1, &(m_Fences[backBufferIndex]));

    VERIFYVULKANRESULT(vkQueueSubmit(m_PresentQueue, 1, &submitInfo, m_Fences[backBufferIndex]));
    
    vkWaitForFences(m_Device, 1, &(m_Fences[backBufferIndex]), true, ((uint64)	0xffffffffffffffff));
    
    // present
    m_SwapChain->Present(*m_VulkanDevice->GetGraphicsQueue(),*m_VulkanDevice->GetPresentQueue() , &m_RenderComplete);
}

int32 VulkanCommandPool::AcquireBackbufferIndex()
{
    int32 backBufferIndex = m_SwapChain->AcquireImageIndex(&m_PresentComplete);
    return backBufferIndex;
}

uint32 VulkanCommandPool::GetMemoryTypeFromProperties(uint32 typeBits, VkMemoryPropertyFlags properties)
{
    uint32 memoryTypeIndex = 0;
   m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(typeBits, properties, &memoryTypeIndex);
    return memoryTypeIndex;
}

void VulkanCommandPool::CreateDefaultRes()
{
    // VulkanCommandBuffer* cmdbuffer = VulkanCommandBuffer::Create(m_Device, m_CommandPool);
    // DVKDefaultRes::Init(m_Device, cmdbuffer);
    // delete cmdbuffer;
}

void VulkanCommandPool::DestroyDefaultRes()
{
}

void VulkanCommandPool::CreateCommandBuffers()
{
    VkDevice device  = m_Device;

    VkCommandPoolCreateInfo cmdPoolInfo;
    ZeroVulkanStruct(cmdPoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
    cmdPoolInfo.queueFamilyIndex = m_VulkanDevice->GetPresentQueue()->GetFamilyIndex();
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VERIFYVULKANRESULT(vkCreateCommandPool(device, &cmdPoolInfo, VULKAN_CPU_ALLOCATOR, &m_CommandPool));

    VkCommandPoolCreateInfo computePoolInfo;
    ZeroVulkanStruct(computePoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
    computePoolInfo.queueFamilyIndex = m_VulkanDevice->GetComputeQueue()->GetFamilyIndex();
    computePoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VERIFYVULKANRESULT(vkCreateCommandPool(device, &computePoolInfo, VULKAN_CPU_ALLOCATOR, &m_ComputeCommandPool));

    VkCommandBufferAllocateInfo cmdBufferInfo;
    ZeroVulkanStruct(cmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    cmdBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferInfo.commandBufferCount = 1;
    cmdBufferInfo.commandPool        = m_CommandPool;

    m_CommandBuffers.resize(m_SwapChain->GetBackBufferCount());
    for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
    {
        vkAllocateCommandBuffers(device, &cmdBufferInfo, &(m_CommandBuffers[i]));
    }
}

void VulkanCommandPool::DestroyCommandBuffers()
{
    VkDevice device = m_Device;
    for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
    {
        vkFreeCommandBuffers(device, m_CommandPool, 1, &(m_CommandBuffers[i]));
    }

    vkDestroyCommandPool(device, m_CommandPool, VULKAN_CPU_ALLOCATOR);
    vkDestroyCommandPool(device, m_ComputeCommandPool, VULKAN_CPU_ALLOCATOR);
}

void VulkanCommandPool::CreateFences()
{
    VkDevice device  = m_Device;
    int32 frameCount = m_SwapChain->GetBackBufferCount();

    VkFenceCreateInfo fenceCreateInfo;
    ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    m_Fences.resize(frameCount);
    for (int32 i = 0; i < m_Fences.size(); ++i)
    {
        VERIFYVULKANRESULT(vkCreateFence(device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
    }

    VkSemaphoreCreateInfo createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
    vkCreateSemaphore(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
}

void VulkanCommandPool::DestroyFences()
{
    VkDevice device = m_Device;

    for (int32 i = 0; i < m_Fences.size(); ++i)
    {
        vkDestroyFence(device, m_Fences[i], VULKAN_CPU_ALLOCATOR);
    }

    vkDestroySemaphore(device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
}

void VulkanCommandPool::DestroyPipelineCache()
{
    VkDevice device = m_Device;
    vkDestroyPipelineCache(device, m_PipelineCache, VULKAN_CPU_ALLOCATOR);
    m_PipelineCache = VK_NULL_HANDLE;
}

void VulkanCommandPool::CreatePipelineCache()
{
    VkDevice device = m_Device;

    VkPipelineCacheCreateInfo createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
    VERIFYVULKANRESULT(vkCreatePipelineCache(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineCache));
}

}

