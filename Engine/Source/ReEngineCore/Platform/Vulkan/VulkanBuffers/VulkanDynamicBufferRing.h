#pragma once
#include "VulkanBuffer.h"
#include "Core/Ring.h"
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanDevice.h"


class VulkanDynamicBufferRing
{
public:
    ~VulkanDynamicBufferRing()
    {
        m_Buffer.reset();
        OnDestroy();
    }
    
    VkResult OnCreate(Ref<VulkanDevice> Device , uint32_t NUmberOfBackBuffers,uint32_t MemTotalSize,char *name = NULL);
    void OnDestroy();
    bool AllocConstantBuffer(uint32_t size,void **pData,VkDescriptorBufferInfo *pOut);
    VkDescriptorBufferInfo AllocConstantBuffer(uint32_t size, void *pData);
    void OnBeginFrame();
    void SetDescriptorSet(int i, uint32_t size, VkDescriptorSet descriptorSet);

private:
    Ref<VulkanDevice> m_Device = nullptr;
    Ref<VulkanBuffer> m_Buffer = nullptr;
    uint32_t m_MemTotalSize;
    RingWithTab m_Mem;
    char *m_pData = nullptr;

#ifdef USE_VMA
    VmaAllocation   m_bufferAlloc = VK_NULL_HANDLE;
#else
    VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
#endif
    
};
