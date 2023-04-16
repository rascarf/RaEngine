#include "VulkanCommandPool.h"
#include "Core/Application.h"
#include "VulkanContext.h"
#include "Renderer/RHI/Renderer.h"

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
    if(backBufferIndex < 0) backBufferIndex = 0;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask    = &m_WaitStageMask;
    submitInfo.pWaitSemaphores      = &m_PresentComplete;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pSignalSemaphores    = &m_RenderComplete[backBufferIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pCommandBuffers      = &(m_CommandBuffers[backBufferIndex]);
    submitInfo.commandBufferCount   = 1;

    //Fence保证在队列没有提交完成之前，cpu不会执行下面的代码，挡住的是CPU，不是GPU，Semaphore才是挡住GPU
    vkResetFences(m_Device, 1, &(m_Fences[m_FrameIndex]));
    VERIFYVULKANRESULT(vkQueueSubmit(m_PresentQueue, 1, &submitInfo, m_Fences[backBufferIndex]));
    vkWaitForFences(m_Device, 1, &(m_Fences[m_FrameIndex]), true, ((uint64)	0xffffffffffffffff));
    
    // present会等待RenderComplete Semaphore
    m_SwapChain->Present(*m_VulkanDevice->GetGraphicsQueue(),*m_VulkanDevice->GetPresentQueue() , &m_RenderComplete[backBufferIndex]);
}

int32 VulkanCommandPool::AcquireBackbufferIndex()
{
    //当前帧的序号
    //这一帧提交等待的semaphore
    //渲染指令提交会固定signal RenderComplete
    //这一帧需要等待这个PresentCompelete才可以渲染
    int32 backBufferIndex = m_SwapChain->AcquireImageIndex(&m_PresentComplete);

    if(backBufferIndex < 0 )
    {
        m_FrameIndex = 0;
        auto Context = Renderer::GetContext().get();
        auto VulkanContext = dynamic_cast<ReEngine::VulkanContext*>(Context);
        VulkanContext-> RecreateSwapChain();
    }

    m_FrameIndex =  backBufferIndex;
    
    return m_FrameIndex;
}

uint32 VulkanCommandPool::GetMemoryTypeFromProperties(uint32 typeBits, VkMemoryPropertyFlags properties)
{
    uint32 memoryTypeIndex = 0;
    m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(typeBits, properties, &memoryTypeIndex);
    return memoryTypeIndex;
}

void VulkanCommandPool::CreateDefaultRes()
{
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

    m_RenderComplete.resize(frameCount);
    for(int32 i = 0; i < m_RenderComplete.size(); ++i)
    {
        VkSemaphoreCreateInfo createInfo;
        ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
        vkCreateSemaphore(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete[i]);
    }
}

void VulkanCommandPool::DestroyFences()
{
    VkDevice device = m_Device;

    for (int32 i = 0; i < m_Fences.size(); ++i)
    {
        vkDestroyFence(device, m_Fences[i], VULKAN_CPU_ALLOCATOR);
    }
    
    for(int32 i = 0; i < m_RenderComplete.size(); ++i)
    {
        vkDestroySemaphore(device, m_RenderComplete[i], VULKAN_CPU_ALLOCATOR);
    }
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

void VulkanCommandPool::RecreateSwapChain()
{
    
}
}

