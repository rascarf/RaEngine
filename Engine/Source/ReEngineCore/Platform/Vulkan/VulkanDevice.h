#pragma once

#include "Core/Core.h"
#include "VulkanCommonDefine.h"

#include "VulkanQueue.h"
#include "VulkanMemory.h"

#include <vector>
#include <memory>
#include <map>

class VulkanFenceManager;
class VulkanDeviceMemoryManager;

class VulkanDevice
{
public:
    VulkanDevice(VkPhysicalDevice physicalDevice);
    
    virtual ~VulkanDevice();
    
    bool QueryGPU(int32 deviceIndex);
    
    void InitGPU(int32 deviceIndex);
    
    void CreateDevice();
    
    void Destroy();
    
    bool IsFormatSupported(VkFormat format);
    
    const VkComponentMapping& GetFormatComponentMapping(ReEngine::PixelFormat format) const;
    
    void SetupPresentQueue(VkSurfaceKHR surface);
    
    FORCE_INLINE Ref<VulkanQueue> GetGraphicsQueue()
    {
        return m_GfxQueue;
    }
    
    FORCE_INLINE Ref<VulkanQueue> GetComputeQueue()
    {
        return m_ComputeQueue;
    }
    
    FORCE_INLINE Ref<VulkanQueue> GetTransferQueue()
    {
        return m_TransferQueue;
    }
    
    FORCE_INLINE Ref<VulkanQueue> GetPresentQueue()
    {
        return m_PresentQueue;
    }
    
    FORCE_INLINE VkPhysicalDevice GetPhysicalHandle() const
    {
        return m_PhysicalDevice;
    }
    
    FORCE_INLINE const VkPhysicalDeviceProperties& GetDeviceProperties() const
    {
        return m_PhysicalDeviceProperties;
    }
    
    FORCE_INLINE const VkPhysicalDeviceLimits& GetLimits() const
    {
        return m_PhysicalDeviceProperties.limits;
    }
    
    FORCE_INLINE const VkPhysicalDeviceFeatures& GetPhysicalFeatures() const
    {
        return m_PhysicalDeviceFeatures;
    }
    
    FORCE_INLINE VkDevice GetInstanceHandle() const
    {
        return m_Device;
    }
    
    FORCE_INLINE const VkFormatProperties* GetFormatProperties() const
    {
        return m_FormatProperties;
    }
    
	FORCE_INLINE VulkanFenceManager& GetFenceManager()
	{
		return *m_FenceManager;
	}
    
    FORCE_INLINE VulkanDeviceMemoryManager& GetMemoryManager()
    {
        return *m_MemoryManager;
    }
    
	FORCE_INLINE void AddAppDeviceExtensions(const char* name)
	{
		m_AppDeviceExtensions.push_back(name);
	}

	FORCE_INLINE void SetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures2* deviceFeatures)
	{
		m_PhysicalDeviceFeatures2 = deviceFeatures;
	}

private:
    
    void MapFormatSupport(ReEngine::PixelFormat format, VkFormat vkFormat);
    
    void MapFormatSupport(ReEngine::PixelFormat format, VkFormat vkormat, int32 blockBytes);
    
    void SetComponentMapping(ReEngine::PixelFormat format, VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a);
    
    void GetDeviceExtensionsAndLayers(std::vector<const char*>& outDeviceExtensions, std::vector<const char*>& outDeviceLayers, bool& bOutDebugMarkers);
    
    void SetupFormats();
    
private:
    friend class VulkanInstance;
    friend class VulkanContext;

private:
    VkDevice                                m_Device;
    VkPhysicalDevice                        m_PhysicalDevice;
    VkPhysicalDeviceProperties              m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures                m_PhysicalDeviceFeatures;
    std::vector<VkQueueFamilyProperties>    m_QueueFamilyProps;
    
    VkFormatProperties                      m_FormatProperties[PixelFormat_MAX];
    std::map<VkFormat, VkFormatProperties>  m_ExtensionFormatProperties;
	VkComponentMapping                      m_PixelFormatComponentMapping[PixelFormat_MAX];

    Ref<VulkanQueue>                        m_GfxQueue;
    Ref<VulkanQueue>                        m_ComputeQueue;
    Ref<VulkanQueue>                        m_TransferQueue;
    Ref<VulkanQueue>                        m_PresentQueue;

    VulkanFenceManager*                     m_FenceManager;
    VulkanDeviceMemoryManager*              m_MemoryManager;

	std::vector<const char*>				m_AppDeviceExtensions;
	VkPhysicalDeviceFeatures2*				m_PhysicalDeviceFeatures2;
};
