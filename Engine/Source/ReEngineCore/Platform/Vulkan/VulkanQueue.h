#pragma once

#include "VulkanCommonDefine.h"

#include <memory>

class VulkanDevice;

class VulkanQueue
{
public:
    
    VulkanQueue(VulkanDevice* device, uint32 familyIndex);
    VulkanQueue():m_Queue(nullptr),m_FamilyIndex(-1),m_Device(nullptr){}
    
    virtual ~VulkanQueue();
    
    FORCE_INLINE int32 GetFamilyIndex() const
    {
        return m_FamilyIndex;
    }
    
    FORCE_INLINE VkQueue GetHandle() const
    {
        return m_Queue;
    }
    
private:
    VkQueue         m_Queue;
    uint32          m_FamilyIndex;
    VulkanDevice*   m_Device;
};

