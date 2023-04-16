#pragma once

#include "Core/Core.h"
#include "VulkanCommonDefine.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include <string>


namespace ReEngine
{
    class VulkanContext;

class VulkanCommandPool
{
public:
    VulkanCommandPool():
        m_Device(VK_NULL_HANDLE)
        , m_GfxQueue(VK_NULL_HANDLE)
        , m_PresentQueue(VK_NULL_HANDLE)
        , m_FrameWidth(0)
        , m_FrameHeight(0)
        , m_PipelineCache(VK_NULL_HANDLE)
        , m_PresentComplete(VK_NULL_HANDLE)
        , m_CommandPool(VK_NULL_HANDLE)
        , m_WaitStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
        , m_SwapChain(VK_NULL_HANDLE)
    {
    }

    virtual ~VulkanCommandPool()
    {}

    VkDevice GetDeviceHandle()
    {
        return m_VulkanDevice->GetInstanceHandle();
    }
    
    virtual void UpdateFPS(float time, float delta)
    {
        m_FrameCounter  += 1;
        m_LastFrameTime += delta;
        if (m_LastFrameTime >= 1.0f)
        {
            m_LastFPS       = m_FrameCounter;
            m_FrameCounter  = 0;
            m_LastFrameTime = 0.0f;
        }
    }

    void Init(VulkanContext* Context);
    
    void ShutDown()
    {
        DestroyDefaultRes();
        DestroyFences();
        DestroyCommandBuffers();
        DestroyPipelineCache();
    }

    void Present(int backBufferIndex);

    int32 AcquireBackbufferIndex();

    uint32 GetMemoryTypeFromProperties(uint32 typeBits, VkMemoryPropertyFlags properties);

    void RecreateSwapChain();

private:

    void CreateDefaultRes();

    void DestroyDefaultRes();

    void CreateCommandBuffers();

    void DestroyCommandBuffers();

    void CreateFences();

    void DestroyFences();

    void DestroyPipelineCache();

    void CreatePipelineCache();

public:

    typedef std::shared_ptr<VulkanSwapChain> VulkanSwapChainRef;
    VkDevice                        m_Device;
    std::shared_ptr<VulkanDevice>   m_VulkanDevice;
    VkQueue                         m_GfxQueue;
    VkQueue                         m_PresentQueue;

    int32                           m_FrameWidth;
    int32                           m_FrameHeight;

    VkPipelineCache                 m_PipelineCache;

    std::vector<VkFence>            m_Fences;
    std::vector<VkSemaphore>        m_RenderComplete;
    VkSemaphore                     m_PresentComplete;

    VkCommandPool                   m_CommandPool;
    VkCommandPool                   m_ComputeCommandPool;
    std::vector<VkCommandBuffer>    m_CommandBuffers;

    VkPipelineStageFlags            m_WaitStageMask;

    VulkanSwapChainRef              m_SwapChain;

    int32                           m_FrameCounter = 0;
    float                           m_LastFrameTime = 0.0f;
    int32                           m_LastFPS = 0;
    
    uint32_t m_FrameIndex;
};
}

